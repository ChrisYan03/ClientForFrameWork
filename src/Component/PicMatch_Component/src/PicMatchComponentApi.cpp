#include "PicMatchComponentApi.h"
#include "QmlBridge/PlayerHostItem.h"
#include "QmlBridge/PicMatchViewModel.h"
#include <QQmlEngine>
#include <QCoreApplication>
#include <QDir>

/** 仅注册 QML 类型；桌面入口与图标由主程序根据 meta_info/manifest.json 调用 addDesktopApp 添加。extern "C" 保证符号名为 PicMatchComponent_Register 便于 QLibrary::resolve。 */
extern "C" PICMATCHCOMPONENT_API void PICMATCHCOMPONENT_CALL PicMatchComponent_Register(QQmlEngine *engine, QObject *appController)
{
    Q_UNUSED(appController);
    if (!engine)
        return;

    // 注册 QML 类型
    qmlRegisterType<PlayerHostItem>("App", 1, 0, "PlayerHostItem");

    // 注册 ViewModel 到 QML（作为单例）
    qmlRegisterSingletonType<PicMatchViewModel>("PicMatchCore", 1, 0, "ViewModel",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine);
            Q_UNUSED(scriptEngine);
            return new PicMatchViewModel();
        });
}
