#include "ComponentRuntimeDispatcher.h"

#include "../../../Interface/IComponent.h"
#include "../../ComponentManager.h"

#include <QMetaObject>
#include <QVariant>

namespace {

QVariantMap queryThemeColors(QObject* appController)
{
    if (!appController)
        return {};
    const QVariant v = appController->property("themeColors");
    if (v.canConvert<QVariantMap>())
        return v.toMap();
    return {};
}

void notifyThemeByIpcLike(ComponentInstanceV2* instance, QObject* appController, int theme)
{
    QObject* runtimeObj = instance ? instance->componentObject : nullptr;
    if (!runtimeObj)
        return;
    const QVariantMap themeColors = queryThemeColors(appController);
    (void)QMetaObject::invokeMethod(
        runtimeObj, "applyTheme",
        Qt::DirectConnection,
        Q_ARG(QVariantMap, themeColors));
    (void)QMetaObject::invokeMethod(
        runtimeObj, "onThemeChanged",
        Qt::DirectConnection,
        Q_ARG(int, theme));
}

void notifyLanguageByIpcLike(ComponentInstanceV2* instance, int language)
{
    QObject* runtimeObj = instance ? instance->componentObject : nullptr;
    if (!runtimeObj)
        return;
    (void)QMetaObject::invokeMethod(
        runtimeObj, "setFrameworkLanguage",
        Qt::DirectConnection,
        Q_ARG(int, language));
    (void)QMetaObject::invokeMethod(
        runtimeObj, "onLanguageChanged",
        Qt::DirectConnection,
        Q_ARG(int, language));
}

} // namespace

RuntimeFlowChannel ComponentRuntimeDispatcher::resolveChannel(const ComponentInstanceV2* instance)
{
    if (!instance)
        return RuntimeFlowChannel::Unknown;
    if (instance->componentType == ComponentType_NativeDll)
        return RuntimeFlowChannel::Dll;
    return RuntimeFlowChannel::Ipc;
}

const char* ComponentRuntimeDispatcher::channelName(RuntimeFlowChannel channel)
{
    switch (channel) {
    case RuntimeFlowChannel::Dll:
        return "dll";
    case RuntimeFlowChannel::Ipc:
        return "ipc";
    default:
        return "unknown";
    }
}

void ComponentRuntimeDispatcher::notifyTheme(ComponentInstanceV2* instance, QObject* appController, int theme)
{
    if (!instance || !instance->componentObject)
        return;

    switch (resolveChannel(instance)) {
    case RuntimeFlowChannel::Dll:
        if (IComponent* component = dynamic_cast<IComponent*>(instance->componentObject))
            component->onThemeChanged(theme);
        break;
    case RuntimeFlowChannel::Ipc:
        notifyThemeByIpcLike(instance, appController, theme);
        break;
    default:
        break;
    }
}

void ComponentRuntimeDispatcher::notifyLanguage(ComponentInstanceV2* instance, int language)
{
    if (!instance || !instance->componentObject)
        return;

    switch (resolveChannel(instance)) {
    case RuntimeFlowChannel::Dll:
        if (IComponent* component = dynamic_cast<IComponent*>(instance->componentObject))
            component->onLanguageChanged(language);
        break;
    case RuntimeFlowChannel::Ipc:
        notifyLanguageByIpcLike(instance, language);
        break;
    default:
        break;
    }
}
