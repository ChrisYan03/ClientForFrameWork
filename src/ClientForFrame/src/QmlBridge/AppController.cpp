#include "AppController.h"
#include "PlayerHostItem.h"
#include "../PicMatchComponent/PicMatchWidget.h"
#include "../Common/StyleManager.h"
#include "PicPlayerApi.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    setStatusText(QString());
    loadThemeColors();
}

static void hexToRgbF(const QString &hex, float &r, float &g, float &b)
{
    r = g = b = 0.15f;
    if (hex.isEmpty() || (hex.size() != 7 && hex.size() != 4))
        return;
    QString h = hex.startsWith(QLatin1Char('#')) ? hex.mid(1) : hex;
    if (h.size() >= 6) {
        bool ok;
        r = h.mid(0, 2).toInt(&ok, 16) / 255.0f;
        if (ok) g = h.mid(2, 2).toInt(&ok, 16) / 255.0f;
        if (ok) b = h.mid(4, 2).toInt(&ok, 16) / 255.0f;
    }
}

void AppController::applyThemeToPicPlayer()
{
    QVariant v = m_themeColors.value(QStringLiteral("contentBackground"));
    QString hex = v.toString();
    float r, g, b;
    hexToRgbF(hex, r, g, b);
    PicPlayer_SetBackgroundColor(r, g, b);
}

void AppController::loadThemeColors()
{
    const QString path = (m_theme == 1) ? QStringLiteral(":/themes/dark.json") : QStringLiteral(":/themes/light.json");
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        file.close();
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            m_themeColors = doc.object().toVariantMap();
            applyThemeToPicPlayer();
            emit themeColorsChanged();
            return;
        }
    }
    // JSON 缺失或解析失败：清空 map，QML 侧已对 themeColors.xxx 做 fallback，仅需保证 themes/*.json 在 qrc 中
    m_themeColors = QVariantMap();
    applyThemeToPicPlayer();
    emit themeColorsChanged();
}

void AppController::setTheme(int theme)
{
    if (m_theme == theme)
        return;
    m_theme = theme;
    StyleManager *sm = StyleManager::instance();
    if (sm) {
        sm->applyTheme(theme == 1 ? StyleManager::DarkTheme : StyleManager::LightTheme);
    }
    loadThemeColors();
    emit themeChanged();
}

AppController::~AppController()
{
    if (m_hostWindow) {
        m_hostWindow->deleteLater();
        m_hostWindow = nullptr;
    }
    m_picMatchWidget = nullptr;
}

void AppController::setStatusText(const QString &text)
{
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
    }
}

void AppController::setHasRunnableComponent(bool on)
{
    if (m_hasRunnableComponent != on) {
        m_hasRunnableComponent = on;
        emit hasRunnableComponentChanged();
    }
}

void AppController::setRunning(bool on)
{
    if (m_isRunning != on) {
        m_isRunning = on;
        emit isRunningChanged();
    }
}

void AppController::registerPicMatchHost(QObject *hostItem)
{
    PlayerHostItem *host = qobject_cast<PlayerHostItem *>(hostItem);
    if (!host)
        return;
    setHasRunnableComponent(true);
    auto trySetPlayer = [this, host]() {
        if (PicMatchWidget *w = host->picMatchWidget()) {
            setPlayer(w, nullptr);
        }
    };
    trySetPlayer();
    connect(host, &PlayerHostItem::widgetReady, this, trySetPlayer, Qt::UniqueConnection);
}

void AppController::unregisterPicMatchHost()
{
    if (m_picMatchWidget)
        m_picMatchWidget->Quit();
    setRunning(false);
    setPlayer(nullptr, nullptr);
    setHasRunnableComponent(false);
}

void AppController::requestBackToDesktop()
{
    emit backToDesktopRequested();
}

void AppController::setPlayer(PicMatchWidget *widget, QWidget *hostWindow)
{
    m_picMatchWidget = widget;
    m_hostWindow = hostWindow;
}

void AppController::start()
{
    setRunning(true);
    setStatusText(QStringLiteral("● 运行中"));
    if (m_hostWindow) {
        m_hostWindow->show();
        m_hostWindow->raise();
    }
    if (m_picMatchWidget)
        m_picMatchWidget->Run();
}

void AppController::stop()
{
    setRunning(false);
    setStatusText(QStringLiteral("● 已停止"));
    if (m_picMatchWidget)
        m_picMatchWidget->Quit();
    if (m_hostWindow)
        m_hostWindow->hide();
}

void AppController::closeApp()
{
    if (m_picMatchWidget)
        m_picMatchWidget->Quit();
    emit requestQuit();
}
