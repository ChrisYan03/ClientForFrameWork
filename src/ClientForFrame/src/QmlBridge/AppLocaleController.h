#ifndef APPLOCALECONTROLLER_H
#define APPLOCALECONTROLLER_H

#include <QObject>
#include "TranslationManager.h"

/**
 * @brief 语言切换：TranslationManager 与 UI 过渡窗口，切换后请求刷新框架侧绑定。
 */
class AppLocaleController : public QObject
{
    Q_OBJECT

public:
    explicit AppLocaleController(QObject *parent = nullptr);

    int currentLanguage() const;
    QString currentLanguageName() const;

    void setLanguage(int language);
    QString getLanguageName(int language) const;

signals:
    void currentLanguageChanged();
    /** 在语言切换延迟回调中发出，供门面刷新列表/标签等绑定 */
    void frameworkBindingsRefreshRequested();
};

#endif // APPLOCALECONTROLLER_H
