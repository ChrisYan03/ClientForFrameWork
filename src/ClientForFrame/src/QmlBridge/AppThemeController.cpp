#include "AppThemeController.h"
#include "../Common/StyleManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>

void AppThemeController::applyThemeToHost(QObject *host)
{
    if (!host)
        return;
    QMetaObject::invokeMethod(host, "applyTheme", Q_ARG(QVariantMap, m_themeColors));
}

void AppThemeController::loadThemeColors()
{
    const QString path = (m_theme == 1) ? QStringLiteral(":/themes/dark.json") : QStringLiteral(":/themes/light.json");
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        file.close();
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            m_themeColors = doc.object().toVariantMap();
            emit themeColorsChanged();
            return;
        }
    }
    m_themeColors = QVariantMap();
    emit themeColorsChanged();
}

AppThemeController::AppThemeController(QObject *parent)
    : QObject(parent)
{
    loadThemeColors();
}

void AppThemeController::setTheme(int theme)
{
    if (m_theme == theme)
        return;
    m_theme = theme;
    StyleManager *sm = StyleManager::instance();
    if (sm)
        sm->applyTheme(theme == 1 ? StyleManager::DarkTheme : StyleManager::LightTheme);
    loadThemeColors();
    emit themeChanged(m_theme);
}
