#include "StyleManager.h"
#include <QDir>
#include "LogUtil.h"

StyleManager* StyleManager::m_instance = nullptr;

StyleManager::StyleManager(QObject *parent)
    : QObject(parent)
    , m_currentTheme(LightTheme)
{
    LOG_INFO("StyleManager constructor called");
}

StyleManager* StyleManager::instance()
{
    LOG_INFO("StyleManager::instance() called");
    if (m_instance == nullptr) {
        LOG_INFO("Creating new StyleManager instance");
        // 确保QApplication已创建后再创建StyleManager实例
        if (!qApp) {
            LOG_ERROR("ERROR: QApplication not initialized yet!");
            return nullptr;
        }
        LOG_INFO("QApplication exists, creating StyleManager with parent");
        m_instance = new StyleManager(qApp);  // 设置QApplication为父对象
        LOG_INFO("StyleManager instance created successfully");
    }
    return m_instance;
}

void StyleManager::applyTheme(ThemeType theme)
{
    LOG_INFO("applyTheme called with theme: {}", theme == DarkTheme ? "Dark" : "Light");
    
    if (!qApp) {
        LOG_ERROR("ERROR: QApplication not available for applying theme");
        return;
    }
    
    m_currentTheme = theme;
    LOG_INFO("Theme set to: {}", theme == DarkTheme ? "Dark" : "Light");
    
    QString styleSheetPath;
    switch(theme) {
        case DarkTheme:
            styleSheetPath = ":/styles/dark_theme.qss";
            LOG_INFO("Using dark theme path: {}", styleSheetPath.toStdString());
            break;
        case LightTheme:
        default:
            styleSheetPath = ":/styles/main_theme.qss";
            LOG_INFO("Using light theme path: {}", styleSheetPath.toStdString());
            break;
    }
    
    LOG_INFO("Calling applyStyleSheet with path: {}", styleSheetPath.toStdString());
    applyStyleSheet(styleSheetPath);
    LOG_INFO("applyTheme completed");
}

void StyleManager::applyStyleSheet(const QString& fileName)
{
    LOG_INFO("applyStyleSheet called with file: {}", fileName.toStdString());
    
    if (!qApp) {
        LOG_ERROR("ERROR: QApplication not available for applying stylesheet");
        return;
    }
    
    QFile file(fileName);
    LOG_INFO("Checking if file exists: {}", fileName.toStdString());
    if (!file.exists()) {
        LOG_ERROR("ERROR: Style sheet file does not exist: {}", fileName.toStdString());
        return;
    }
    LOG_INFO("File exists, attempting to open");
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_INFO("File opened successfully, reading content");
        QString styleSheet = QLatin1String(file.readAll());
        LOG_INFO("Content read, applying to QApplication");
        qApp->setStyleSheet(styleSheet);
        LOG_INFO("Stylesheet applied successfully");
        file.close();
    } else {
        LOG_ERROR("ERROR: Cannot open style sheet file: {}", fileName.toStdString());
    }
}

QString StyleManager::loadStyleSheet(const QString& fileName)
{
    LOG_INFO("loadStyleSheet called with file: {}", fileName.toStdString());
    
    QFile file(fileName);
    if (!file.exists()) {
        LOG_ERROR("Style sheet file does not exist: {}", fileName.toStdString());
        return QString();
    }
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        file.close();
        LOG_INFO("Stylesheet loaded successfully");
        return styleSheet;
    } else {
        LOG_ERROR("Cannot open style sheet file: {}", fileName.toStdString());
        return QString();
    }
}