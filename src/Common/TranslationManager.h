#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QTranslator>
#include <QObject>
#include <QString>
#include <QCoreApplication>

/**
 * @brief 翻译管理器，负责应用多语言翻译
 */
class TranslationManager : public QObject
{
    Q_OBJECT

public:
    enum Language {
        Chinese = 0,    // 中文
        English = 1     // English
    };
    Q_ENUM(Language)

    static TranslationManager* instance();

    /**
     * @brief 获取当前语言
     */
    Language currentLanguage() const { return m_currentLanguage; }

    /**
     * @brief 设置语言
     * @param language 语言类型
     */
    void setLanguage(Language language);

    /**
     * @brief 获取语言的显示名称
     */
    QString getLanguageDisplayName(Language language) const;

    /**
     * @brief 从配置文件加载保存的语言设置
     */
    void loadSavedLanguage();

signals:
    void languageChanged(Language language);

public:
    // 语言/主题切换开始时调用，防止 PlayerHostItem 等组件在切换期间访问已销毁对象
    static void beginUiTransition();
    // 语言/主题切换结束后调用
    static void endUiTransition();
    static bool isInUiTransition() { return s_inUiTransition; }

private:
    explicit TranslationManager(QObject *parent = nullptr);
    ~TranslationManager();

    void applyTranslation(Language language);
    void saveLanguage(Language language);
    void loadComponentTranslations();

    QTranslator m_translator;
    QList<QTranslator*> m_componentTranslators;  // 跟踪组件翻译器，用于切换语言时清理
    Language m_currentLanguage;
    QString m_configPath;
    bool m_inSetLanguage = false;  // 防重入标志

    inline static bool s_inUiTransition = false;  // 静态标志，所有实例共享
};

#endif // TRANSLATIONMANAGER_H
