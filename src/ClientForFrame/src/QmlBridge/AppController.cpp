#include "AppController.h"
#include "../Common/StyleManager.h"
#include "TranslationManager.h"
#include <QFile>
#include "LogUtil.h"
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QCoreApplication>
#include <QTimer>

static const QString kDefaultPageTitle(QStringLiteral("小闫客户端"));
static const QString kDefaultPageTitleKey(QStringLiteral("小闫客户端")); // 用于翻译的key

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    setStatusText(QString());
    // 使用翻译后的默认标题
    m_pageTitle = qApp->translate("MainWindow", kDefaultPageTitleKey.toUtf8().constData());
    loadThemeColors();
}

void AppController::applyThemeToPicPlayer()
{
    QObject* host = m_componentHost;
    if (!host)
        return;
    QMetaObject::invokeMethod(host, "applyTheme", Q_ARG(QVariantMap, m_themeColors));
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
    emit themeChanged(m_theme);
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
    // 仅当析构对象仍是当前宿主时才清空。否则：先 pop 再快速 push 时，旧 PlayerHostItem 的 destroyed
    // 可能晚于新页 registerComponentHost，若无条件 m_componentHost=nullptr 会误清新宿主，随后 invokeMethod 等对坏指针有风险。
    QObject::connect(hostItem, &QObject::destroyed, this, [this, hostItem]() {
        if (m_componentHost == hostItem) {
            m_componentHost = nullptr;
            setHasRunnableComponent(false);
        }
    });
    applyThemeToPicPlayer(); // 组件注册后立即应用当前主题，使右侧面板等与主框架换肤一致
}

void AppController::unregisterComponentHost()
{
    QObject* host = m_componentHost;
    if (host) {
        m_componentHost = nullptr;
        QMetaObject::invokeMethod(host, "quit", Qt::DirectConnection);
    }
    setRunning(false);
    setHasRunnableComponent(false);
    // 使用翻译后的默认标题
    setPageTitle(qApp->translate("MainWindow", kDefaultPageTitleKey.toUtf8().constData()));
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
    if (!appId.isEmpty() && !iconPath.isEmpty()) {
        // 检查图标文件是否存在
        if (QFile::exists(iconPath)) {
            m_componentIconPaths.insert(appId, iconPath);
            LOG_INFO("AppController: Registered icon for {} -> {}", appId.toStdString(), iconPath.toStdString());
        } else {
            LOG_WARN("AppController: Icon file not found: {}", iconPath.toStdString());
            // 即使文件不存在也记录空路径，让 QML 使用回退图标
            m_componentIconPaths.insert(appId, QString());
        }
    }
}

void AppController::registerComponentName(const QString &appId, const QString &name)
{
    if (!appId.isEmpty() && !name.isEmpty()) {
        m_componentNames.insert(appId, name);
        LOG_INFO("AppController: Registered name for {} -> {}", appId.toStdString(), name.toStdString());
    }
}

QString AppController::getComponentIconPath(const QString &appId) const
{
    QString path = m_componentIconPaths.value(appId, QString());
    return path.isEmpty() ? path : QUrl::fromLocalFile(path).toString();
}

QString AppController::getComponentName(const QString &appId) const
{
    QString name = m_componentNames.value(appId, QString());
    if (name.isEmpty())
        return QString();
    // 翻译组件名称（同一 context = PicMatchComponent）
    return qApp->translate("PicMatchComponent", name.toUtf8().constData());
}

void AppController::registerComponentPage(const QString &appId, const QUrl &pageUrl)
{
    if (!appId.isEmpty() && pageUrl.isValid()) {
        m_componentPageUrls.insert(appId, pageUrl);
        emit loadedComponentsChanged();
        emit componentCountChanged();
    }
}

QUrl AppController::getComponentPageUrl(const QString &appId) const
{
    return m_componentPageUrls.value(appId, QUrl());
}

void AppController::requestShowBubbleMessage(const QString &message)
{
    if (!message.isEmpty())
        emit showBubbleMessageRequested(message);
}

void AppController::setLanguage(int language)
{
    // 语言/主题切换开始：通知 PlayerHostItem 等组件在延迟回调中跳过操作
    TranslationManager::beginUiTransition();

    LOG_INFO("AppController::setLanguage: language = {}", language);
    TranslationManager::Language lang = static_cast<TranslationManager::Language>(language);
    TranslationManager::instance()->setLanguage(lang);

    // 切换结束后通知组件可以恢复操作（用 QTimer::singleShot 延迟到当前事件处理完成后）
    QTimer::singleShot(0, this, [this]() {
        emit currentLanguageChanged();
        retranslateUi();
        TranslationManager::endUiTransition();
    });
}

QString AppController::getLanguageName(int language) const
{
    TranslationManager::Language lang = static_cast<TranslationManager::Language>(language);
    return TranslationManager::instance()->getLanguageDisplayName(lang);
}

void AppController::retranslateUi()
{
    // 触发QML重新翻译：重新发送所有相关的change信号
    emit pageTitleChanged();
    // 触发组件列表重新绑定，使 getComponentName 被重新调用
    emit loadedComponentsChanged();
    emit componentTabsChanged();
}

QVariantList AppController::componentTabs() const
{
    QVariantList tabs;
    // 只显示已打开的标签
    for (const QString &appId : m_openedTabs) {
        QVariantMap tab;
        tab[QStringLiteral("appId")] = appId;
        tab[QStringLiteral("name")] = getComponentName(appId);
        tab[QStringLiteral("iconPath")] = getComponentIconPath(appId);
        tab[QStringLiteral("isActive")] = (appId == m_currentTabAppId);
        tab[QStringLiteral("isOpened")] = true;
        tabs.append(tab);
    }
    return tabs;
}

int AppController::openComponentTab(const QString &appId)
{
    if (appId.isEmpty() || !m_componentPageUrls.contains(appId))
        return -1;

    // 如果标签已存在，直接切换
    if (m_openedTabs.contains(appId)) {
        switchToTab(appId);
        return m_openedTabs.indexOf(appId);
    }

    // 添加新标签
    m_openedTabs.append(appId);
    m_currentTabAppId = appId;
    emit componentTabsChanged();
    emit currentTabChanged(appId);
    return m_openedTabs.size() - 1;
}

void AppController::closeComponentTab(const QString &appId)
{
    int idx = m_openedTabs.indexOf(appId);
    if (idx < 0)
        return;

    bool wasActive = (appId == m_currentTabAppId);
    m_openedTabs.removeAt(idx);

    if (wasActive) {
        // 如果关闭的是当前标签，切换到前一个或后一个
        if (!m_openedTabs.isEmpty()) {
            int newIdx = qMin(idx, m_openedTabs.size() - 1);
            m_currentTabAppId = m_openedTabs[newIdx];
        } else {
            m_currentTabAppId.clear();
        }
        emit currentTabChanged(m_currentTabAppId);
    }
    emit componentTabsChanged();
}

void AppController::switchToTab(const QString &appId)
{
    if (appId == m_currentTabAppId || !m_openedTabs.contains(appId))
        return;
    m_currentTabAppId = appId;
    emit componentTabsChanged();
    emit currentTabChanged(appId);
}

QString AppController::currentTabAppId() const
{
    return m_currentTabAppId;
}
