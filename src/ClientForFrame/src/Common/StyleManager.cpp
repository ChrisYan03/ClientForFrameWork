#include "StyleManager.h"
#include <QDir>
#include "LogUtil.h"

StyleManager* StyleManager::m_instance = nullptr;

StyleManager::StyleManager(QObject *parent)
    : QObject(parent)
    , m_currentTheme(LightTheme)
{
}

StyleManager* StyleManager::instance()
{
    if (!m_instance) {
        m_instance = new StyleManager();
    }
    return m_instance;
}

void StyleManager::applyTheme(ThemeType theme)
{
    m_currentTheme = theme;
    
    QString styleSheetPath;
    switch(theme) {
        case DarkTheme:
            styleSheetPath = ":/styles/dark_theme.qss";
            break;
        case LightTheme:
        default:
            styleSheetPath = ":/styles/main_theme.qss";
            break;
    }
    
    applyStyleSheet(styleSheetPath);
}

void StyleManager::applyStyleSheet(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists()) {
        LOG_ERROR("Style sheet file does not exist: {}", fileName.toStdString().c_str());
        return;
    }
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    } else {
        LOG_ERROR("Cannot open style sheet file: {}", fileName.toStdString().c_str());
    }
}

QString StyleManager::loadStyleSheet(const QString& fileName)
{
    QFile file(fileName);
    if (!file.exists()) {
        LOG_ERROR("Style sheet file does not exist: {}", fileName.toStdString().c_str());
        return QString();
    }
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        file.close();
        return styleSheet;
    } else {
        LOG_ERROR("Cannot open style sheet file: {}", fileName.toStdString().c_str());
        return QString();
    }
}