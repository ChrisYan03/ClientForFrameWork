#include "StyleManager.h"
#include <QApplication>
#include "LogUtil.h"

StyleManager* StyleManager::m_instance = nullptr;

StyleManager::StyleManager(QObject *parent)
    : QObject(parent)
    , m_currentTheme(LightTheme)
{
}

StyleManager* StyleManager::instance()
{
    if (m_instance == nullptr && qApp) {
        m_instance = new StyleManager(qApp);
    }
    return m_instance;
}

void StyleManager::applyTheme(ThemeType theme)
{
    m_currentTheme = theme;
    LOG_INFO("applyTheme: {}", theme == DarkTheme ? "Dark" : "Light");
    // 主框架为 QML，实际主题由 AppThemeController 读 themes/*.json，经 AppController.themeColors 下发
}