#include "PicMatchComponentApi.h"
#include "QmlBridge/PlayerHostItem.h"
#include "QmlBridge/PicMatchViewModel.h"
#include "QmlBridge/FaceImageProvider.h"
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
    // 可实例化类型，供 PicMatchPage.qml 中 PicMatchViewModel { id: picMatchViewModel } 使用
    qmlRegisterType<PicMatchViewModel>("PicMatchCore", 1, 0, "PicMatchViewModel");

    // 仅注册一次图片 provider，供 QML 通过 image://picmatchfaces/<faceId> 取图。
    if (!engine->property("picmatchFacesProviderInstalled").toBool()) {
        engine->addImageProvider(QStringLiteral("picmatchfaces"), new FaceImageProvider());
        engine->setProperty("picmatchFacesProviderInstalled", true);
    }
}
