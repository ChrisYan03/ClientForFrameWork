#ifndef PICMATCH_COMPONENT_API_H
#define PICMATCH_COMPONENT_API_H

#include "PicMatchComponentGlobal.h"
#include <QQmlEngine>

class QObject;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 当配置启用 PicMatch 组件时，由主程序调用以注册 QML 类型并登记桌面应用。
 * engine: QML 引擎；appController: 框架的 AppController 实例（需已实现 addDesktopApp、registerComponentHost 等）。
 *
 * @deprecated 建议使用 createComponent() 函数通过 IComponent 接口加载
 */
PICMATCHCOMPONENT_API void PICMATCHCOMPONENT_CALL PicMatchComponent_Register(QQmlEngine *engine, QObject *appController);

/**
 * 组件工厂函数 - 通过 IComponent 接口加载
 *
 * 导出此函数供 FrameworkComponentLoader / Framework API 加载组件。
 * 注意：由于 IComponent 定义在主框架中，此函数返回 void* 以避免编译依赖。
 * @return 组件实例指针 (void*)
 */
PICMATCHCOMPONENT_API void* PICMATCHCOMPONENT_CALL createComponent();

#ifdef __cplusplus
}
#endif

#endif // PICMATCH_COMPONENT_API_H
