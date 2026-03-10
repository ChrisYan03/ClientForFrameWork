// Copyright 2025 ClientForFrameWork. Crashpad 初始化，进程内尽早调用。

#ifndef CRASHPADINIT_H
#define CRASHPADINIT_H

#include <QString>

namespace Common {

//! 使用当前可执行目录下的 crashpad_handler 与数据库目录初始化 Crashpad。
//! 应在 QCoreApplication 创建后尽早调用（以便 applicationDirPath 可用）。
//! \param handlerDir 放置 crashpad_handler 的目录，通常为 QCoreApplication::applicationDirPath()
//! \param databaseDir 存放 minidump 的目录，若为空则使用 handlerDir + "/Crashpad"
//! \return 是否初始化成功
bool initializeCrashpad(const QString& handlerDir, const QString& databaseDir = QString());

} // namespace Common

#endif // CRASHPADINIT_H
