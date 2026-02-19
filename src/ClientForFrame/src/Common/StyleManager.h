#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QApplication>

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
    void applyStyleSheet(const QString& fileName);
    QString loadStyleSheet(const QString& fileName);

private:
    static StyleManager* m_instance;
    ThemeType m_currentTheme;
};

#endif // STYLEMANAGER_H