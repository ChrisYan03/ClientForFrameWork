#ifndef APPTHEMECONTROLLER_H
#define APPTHEMECONTROLLER_H

#include <QObject>
#include <QVariantMap>

/**
 * @brief 主题索引、themeColors JSON 加载，并向组件宿主下发 applyTheme。
 */
class AppThemeController : public QObject
{
    Q_OBJECT

public:
    explicit AppThemeController(QObject *parent = nullptr);

    int theme() const { return m_theme; }
    QVariantMap themeColors() const { return m_themeColors; }

    void setTheme(int theme);
    void applyThemeToHost(QObject *host);

signals:
    void themeChanged(int theme);
    void themeColorsChanged();

private:
    void loadThemeColors();

    int m_theme = 0;
    QVariantMap m_themeColors;
};

#endif // APPTHEMECONTROLLER_H
