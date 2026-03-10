#include "AppController.h"
#include "../Common/StyleManager.h"
#include <QFile>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>

static const QString kDefaultPageTitle(QStringLiteral("小闫客户端"));

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    setStatusText(QString());
    m_pageTitle = kDefaultPageTitle;
    loadThemeColors();
}

void AppController::applyThemeToPicPlayer()
{
    if (m_componentHost)
        QMetaObject::invokeMethod(m_componentHost, "applyTheme", Q_ARG(QVariantMap, m_themeColors));
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
    m_componentHost = nullptr;
}

void AppController::setStatusText(const QString &text)
{
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
    }
}

void AppController::setPageTitle(const QString &title)
{
    if (m_pageTitle != title) {
        m_pageTitle = title;
        emit pageTitleChanged();
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

void AppController::registerComponentHost(QObject *hostItem)
{
    if (!hostItem)
        return;
    m_componentHost = hostItem;
    setHasRunnableComponent(true);
    QObject::connect(hostItem, &QObject::destroyed, this, [this]() { m_componentHost = nullptr; setHasRunnableComponent(false); });
    applyThemeToPicPlayer(); // 组件注册后立即应用当前主题，使右侧面板等与主框架换肤一致
}

void AppController::unregisterComponentHost()
{
    if (m_componentHost) {
        QMetaObject::invokeMethod(m_componentHost, "quit", Qt::DirectConnection);
        m_componentHost = nullptr;
    }
    setRunning(false);
    setHasRunnableComponent(false);
    setPageTitle(kDefaultPageTitle);
}

void AppController::requestBackToDesktop()
{
    emit backToDesktopRequested();
}

void AppController::start()
{
    setRunning(true);
    if (m_componentHost)
        QMetaObject::invokeMethod(m_componentHost, "run", Qt::DirectConnection);
}

void AppController::stop()
{
    setRunning(false);
    if (m_componentHost)
        QMetaObject::invokeMethod(m_componentHost, "quit", Qt::DirectConnection);
}

void AppController::closeApp()
{
    if (m_componentHost)
        QMetaObject::invokeMethod(m_componentHost, "quit", Qt::DirectConnection);
    emit requestQuit();
}

void AppController::registerComponentIcon(const QString &appId, const QString &iconPath)
{
    if (!appId.isEmpty() && !iconPath.isEmpty())
        m_componentIconPaths.insert(appId, iconPath);
}

QString AppController::getComponentIconPath(const QString &appId) const
{
    QString path = m_componentIconPaths.value(appId, QString());
    return path.isEmpty() ? path : QUrl::fromLocalFile(path).toString();
}

void AppController::registerComponentPage(const QString &appId, const QUrl &pageUrl)
{
    if (!appId.isEmpty() && pageUrl.isValid())
        m_componentPageUrls.insert(appId, pageUrl);
}

QUrl AppController::getComponentPageUrl(const QString &appId) const
{
    return m_componentPageUrls.value(appId, QUrl());
}
