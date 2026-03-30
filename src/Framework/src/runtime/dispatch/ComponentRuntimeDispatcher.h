#ifndef COMPONENT_RUNTIME_DISPATCHER_H
#define COMPONENT_RUNTIME_DISPATCHER_H

#include "../../../include/main/ComponentTypes.h"

class QObject;
class ComponentInstanceV2;

enum class RuntimeFlowChannel {
    Dll,
    Ipc,
    Unknown
};

class ComponentRuntimeDispatcher
{
public:
    static RuntimeFlowChannel resolveChannel(const ComponentInstanceV2* instance);
    static const char* channelName(RuntimeFlowChannel channel);

    static void notifyTheme(ComponentInstanceV2* instance, QObject* appController, int theme);
    static void notifyLanguage(ComponentInstanceV2* instance, int language);
};

#endif // COMPONENT_RUNTIME_DISPATCHER_H
