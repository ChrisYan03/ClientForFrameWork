#ifndef COMPONENTLOADER_H
#define COMPONENTLOADER_H

#include "ApplicationPaths.h"
#include <QPair>
#include <QString>
#include <QStringList>

class QQmlEngine;
class QObject;

/** 退出时调用组件的 shutdown：库路径 + 符号名 */
using ShutdownEntry = QPair<QString, QString>;

/**
 * @brief 组件加载器：读取 config 启用的组件 ID，加载 DLL 并调用 Register，收集 Shutdown 条目。
 */
class ComponentLoader
{
public:
    ComponentLoader();

    /** 从 config 目录读取启用的组件 ID 列表（先 components.json，否则 component.json）；可能更新 baseDir 为 Release */
    QStringList loadEnabledComponents();

    /** 加载单个组件 DLL，调用 Register(engine, appController)，向 shutdownList 追加 Shutdown 条目 */
    void loadComponentDll(QQmlEngine *engine, QObject *appController, const QString &componentId, QList<ShutdownEntry> *shutdownList);

    /** 加载所有启用组件，返回退出时需调用的 Shutdown 列表 */
    QList<ShutdownEntry> loadAllComponents(QQmlEngine *engine, QObject *appController);

    /** 应用退出时调用，执行 shutdownList 中的每个符号 */
    static void runShutdownList(const QList<ShutdownEntry> &shutdownList);

    const ApplicationPaths &paths() const { return m_paths; }

private:
    ApplicationPaths m_paths;
};

#endif // COMPONENTLOADER_H
