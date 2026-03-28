#include "AppLocaleController.h"
#include <QTimer>
#include "LogUtil.h"

AppLocaleController::AppLocaleController(QObject *parent)
    : QObject(parent)
{
}

int AppLocaleController::currentLanguage() const
{
    return static_cast<int>(TranslationManager::instance()->currentLanguage());
}

QString AppLocaleController::currentLanguageName() const
{
    return TranslationManager::instance()->getLanguageDisplayName(
        TranslationManager::instance()->currentLanguage());
}

void AppLocaleController::setLanguage(int language)
{
    TranslationManager::beginUiTransition();

    LOG_INFO("AppLocaleController::setLanguage: language = {}", language);
    const TranslationManager::Language lang = static_cast<TranslationManager::Language>(language);
    TranslationManager::instance()->setLanguage(lang);

    QTimer::singleShot(0, this, [this]() {
        emit currentLanguageChanged();
        emit frameworkBindingsRefreshRequested();
        TranslationManager::endUiTransition();
    });
}

QString AppLocaleController::getLanguageName(int language) const
{
    const TranslationManager::Language lang = static_cast<TranslationManager::Language>(language);
    return TranslationManager::instance()->getLanguageDisplayName(lang);
}
