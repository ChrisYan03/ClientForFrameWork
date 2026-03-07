#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <QObject>

/** 主框架为 QML 工程，主题由 themes/light.json、dark.json 与 appController.themeColors 驱动，此处仅记录当前主题类型供 setTheme 使用。 */
class StyleManager : public QObject
{
    Q_OBJECT

public:
    enum ThemeType {
        LightTheme,
        DarkTheme
    };

    explicit StyleManager(QObject *parent = nullptr);
    static StyleManager* instance();

    void applyTheme(ThemeType theme);
    ThemeType currentTheme() const { return m_currentTheme; }

private:
    static StyleManager* m_instance;
    ThemeType m_currentTheme;
};

#endif // STYLEMANAGER_H