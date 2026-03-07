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
 */
PICMATCHCOMPONENT_API void PICMATCHCOMPONENT_CALL PicMatchComponent_Register(QQmlEngine *engine, QObject *appController);

#ifdef __cplusplus
}
#endif

#endif // PICMATCH_COMPONENT_API_H
