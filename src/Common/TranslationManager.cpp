#include "TranslationManager.h"
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QFileInfo>

TranslationManager* TranslationManager::instance()
{
    static TranslationManager instance;
    return &instance;
}

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent)
    , m_currentLanguage(Chinese)  // 默认中文
    , m_configPath("config.ini")
{
}

TranslationManager::~TranslationManager()
{
}

void TranslationManager::setLanguage(Language language)
{
    if (m_currentLanguage == language) {
        return;  // 语言未变化，无需处理
    }

    m_currentLanguage = language;
    applyTranslation(language);
    saveLanguage(language);
    emit languageChanged(language);
}

void TranslationManager::applyTranslation(Language language)
{
    // 移除所有已安装的翻译器
    QCoreApplication::removeTranslator(&m_translator);

    // 移除并清理组件翻译器
    for (QTranslator* trans : m_componentTranslators) {
        QCoreApplication::removeTranslator(trans);
        delete trans;
    }
    m_componentTranslators.clear();

    // 加载主框架翻译文件
    QString mainTranslationFile;
    if (language == Chinese) {
        mainTranslationFile = "clientframe_zh_CN.qm";
    } else {
        mainTranslationFile = "clientframe_en_US.qm";
    }

    // 主框架翻译文件路径：仅从translations目录
    QString mainTranslationPath = QCoreApplication::applicationDirPath() + "/translations/" + mainTranslationFile;
    if (m_translator.load(mainTranslationPath)) {
        QCoreApplication::installTranslator(&m_translator);
        qDebug() << "Translation loaded:" << mainTranslationPath;
    } else {
        qWarning() << "Failed to load main translation file:" << mainTranslationPath;
    }

    // 加载组件翻译文件（自动扫描Component目录）
    loadComponentTranslations();
}

void TranslationManager::loadComponentTranslations()
{
    // 扫描Component目录下的所有meta_info/runtime_info/language目录
    QString appDir = QCoreApplication::applicationDirPath();
#ifdef Q_OS_MAC
    // macOS: appDir = Contents/MacOS/，Component/ 在 bin/ 同��
    if (appDir.endsWith("/MacOS") || appDir.endsWith("/macOS")) {
        // 直接移除 /ClientForFrame.app/Contents/MacOS 后缀
        QString suffix = "/ClientForFrame.app/Contents/MacOS";
        if (appDir.endsWith(suffix)) {
            appDir = appDir.left(appDir.length() - suffix.length());
        }
        qDebug() << "macOS: Adjusted appDir to:" << appDir;
    }
#endif
    QString componentBaseDir = appDir + "/Component";
    qDebug() << "Loading component translations from:" << componentBaseDir << "language:" << m_currentLanguage;
    QDir componentDir(componentBaseDir);

    // 根据当前语言确定翻译文件后缀
    QString langSuffix;
    if (m_currentLanguage == Chinese) {
        langSuffix = "_zh_CN.qm";
    } else {
        langSuffix = "_en_US.qm";
    }
    qDebug() << "Looking for translation files with suffix:" << langSuffix;

    if (componentDir.exists()) {
        QFileInfoList componentDirs = componentDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo &dirInfo : componentDirs) {
            QString languageDir = dirInfo.absoluteFilePath() + "/meta_info/runtime_info/language";
            QDir langDir(languageDir);
            if (langDir.exists()) {
                // 只加载匹配当前语言的翻译文件
                QString componentName = dirInfo.fileName().toLower();
                QString qmFile = langDir.path() + "/" + componentName + langSuffix;
                qDebug() << "Trying to load component translation:" << qmFile;
                QTranslator *translator = new QTranslator();
                if (QFile::exists(qmFile)) {
                    if (translator->load(qmFile)) {
                        QCoreApplication::installTranslator(translator);
                        m_componentTranslators.append(translator);
                        qDebug() << "Component translation loaded:" << qmFile;
                        // 验证翻译是否正确安装
                        QString test = qApp->translate("PicMatchToolBar", "启动");
                        qDebug() << "Test translation '启动' ->" << test;
                    } else {
                        qWarning() << "Failed to load component translation (load failed):" << qmFile;
                        delete translator;
                    }
                } else {
                    qWarning() << "Component translation file does not exist:" << qmFile;
                    delete translator;
                }
            }
        }
    }
}

QString TranslationManager::getLanguageDisplayName(Language language) const
{
    switch (language) {
    case Chinese:
        return "中文";
    case English:
        return "English";
    default:
        return "中文";
    }
}

void TranslationManager::saveLanguage(Language language)
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue("language", static_cast<int>(language));
}

void TranslationManager::loadSavedLanguage()
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    if (settings.contains("language")) {
        int languageValue = settings.value("language").toInt();
        m_currentLanguage = static_cast<Language>(languageValue);
        applyTranslation(m_currentLanguage);
    } else {
        // 如果没有保存的设置，使用默认中文
        applyTranslation(m_currentLanguage);
    }
}
