#include "AppController.h"
#include "AppLocaleController.h"
#include "AppThemeController.h"
#include "ComponentRegistry.h"
#include "ComponentTabManager.h"
#include <QCoreApplication>
#include <QMetaObject>
#include "LogUtil.h"

static const QString kDefaultPageTitleKey(QStringLiteral("小闫客户端"));

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_registry(new ComponentRegistry(this))
    , m_tabs(new ComponentTabManager(m_registry, this))
    , m_theme(new AppThemeController(this))
    , m_locale(new AppLocaleController(this))
{
    setStatusText(QString());
    m_pageTitle = qApp->translate("MainWindow", kDefaultPageTitleKey.toUtf8().constData());

    connect(m_locale, &AppLocaleController::currentLanguageChanged,
            this, &AppController::currentLanguageChanged);
    connect(m_locale, &AppLocaleController::frameworkBindingsRefreshRequested,
            this, &AppController::refreshFrameworkBindingsAfterLocaleChange);

    connect(m_registry, &ComponentRegistry::loadedComponentsChanged,
            this, &AppController::loadedComponentsChanged);
    connect(m_registry, &ComponentRegistry::componentCountChanged,
            this, &AppController::componentCountChanged);
    connect(m_tabs, &ComponentTabManager::componentTabsChanged,
            this, &AppController::componentTabsChanged);
    connect(m_tabs, &ComponentTabManager::currentTabChanged,
            this, &AppController::currentTabChanged);
    connect(m_theme, &AppThemeController::themeChanged,
            this, &AppController::themeChanged);
    connect(m_theme, &AppThemeController::themeColorsChanged,
            this, &AppController::themeColorsChanged);
    connect(m_theme, &AppThemeController::themeColorsChanged,
            this, &AppController::applyThemeToCurrentHost);
}

AppController::~AppController()
{
    m_componentHost = nullptr;
}

int AppController::theme() const
{
    return m_theme ? m_theme->theme() : 0;
}

QVariantMap AppController::themeColors() const
{
    return m_theme ? m_theme->themeColors() : QVariantMap();
}

QStringList AppController::loadedComponents() const
{
    return m_registry ? m_registry->loadedComponentIds() : QStringList();
}

int AppController::componentCount() const
{
    return m_registry ? m_registry->componentCount() : 0;
}

QVariantList AppController::componentTabs() const
{
    return m_tabs ? m_tabs->componentTabs() : QVariantList();
}

int AppController::currentLanguage() const
{
    return m_locale ? m_locale->currentLanguage() : 0;
}

QString AppController::currentLanguageName() const
{
    return m_locale ? m_locale->currentLanguageName() : QString();
}

void AppController::applyThemeToCurrentHost()
{
    if (m_theme && m_componentHost)
        m_theme->applyThemeToHost(m_componentHost);
}

void AppController::setTheme(int theme)
{
    if (m_theme)
        m_theme->setTheme(theme);
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
    QObject::connect(hostItem, &QObject::destroyed, this, [this, hostItem]() {
        if (m_componentHost == hostItem) {
            m_componentHost = nullptr;
            setHasRunnableComponent(false);
        }
    });
    applyThemeToCurrentHost();
}

void AppController::setComponentRuntimeActive(bool active)
{
    setRunning(active);
}

void AppController::unregisterComponentHost()
{
    QObject *host = m_componentHost;
    if (host) {
        m_componentHost = nullptr;
        QMetaObject::invokeMethod(host, "quit", Qt::DirectConnection);
    }
    setRunning(false);
    setHasRunnableComponent(false);
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
    if (m_registry)
        m_registry->registerComponentIcon(appId, iconPath);
}

void AppController::registerComponentName(const QString &appId, const QString &name)
{
    if (m_registry)
        m_registry->registerComponentName(appId, name);
}

QString AppController::getComponentIconPath(const QString &appId) const
{
    return m_registry ? m_registry->getComponentIconUrl(appId) : QString();
}

QString AppController::getComponentName(const QString &appId) const
{
    return m_registry ? m_registry->getComponentName(appId) : QString();
}

void AppController::registerComponentPage(const QString &appId, const QUrl &pageUrl)
{
    if (m_registry)
        m_registry->registerComponentPage(appId, pageUrl);
}

QUrl AppController::getComponentPageUrl(const QString &appId) const
{
    return m_registry ? m_registry->getComponentPageUrl(appId) : QUrl();
}

void AppController::requestShowBubbleMessage(const QString &message)
{
    if (!message.isEmpty())
        emit showBubbleMessageRequested(message);
}

void AppController::setLanguage(int language)
{
    if (m_locale)
        m_locale->setLanguage(language);
}

QString AppController::getLanguageName(int language) const
{
    return m_locale ? m_locale->getLanguageName(language) : QString();
}

void AppController::retranslateUi()
{
    refreshFrameworkBindingsAfterLocaleChange();
}

void AppController::refreshFrameworkBindingsAfterLocaleChange()
{
    emit pageTitleChanged();
    emit loadedComponentsChanged();
    emit componentTabsChanged();
}

int AppController::openComponentTab(const QString &appId)
{
    return m_tabs ? m_tabs->openComponentTab(appId) : -1;
}

void AppController::closeComponentTab(const QString &appId)
{
    if (m_tabs)
        m_tabs->closeComponentTab(appId);
}

void AppController::switchToTab(const QString &appId)
{
    if (m_tabs)
        m_tabs->switchToTab(appId);
}

QString AppController::currentTabAppId() const
{
    return m_tabs ? m_tabs->currentTabAppId() : QString();
}
