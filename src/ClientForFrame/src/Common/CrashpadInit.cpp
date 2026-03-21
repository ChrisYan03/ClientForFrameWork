// Copyright 2025 ClientForFrameWork. Crashpad 初始化实现。

#include "CrashpadInit.h"
#include "LogUtil.h"
#include <QDir>
#include <QStandardPaths>

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include "client/crashpad_client.h"
#include "base/files/file_path.h"
#include <string>
#include <map>
#include <vector>
#endif

namespace Common {

bool initializeCrashpad(const QString& handlerDir, const QString& databaseDir)
{
#if !defined(Q_OS_MAC) && !defined(Q_OS_WIN)
    Q_UNUSED(handlerDir);
    Q_UNUSED(databaseDir);
    return false;
#else

    QString handlerPath;
#if defined(Q_OS_MAC)
    handlerPath = handlerDir + QStringLiteral("/crashpad_handler");
#else
    handlerPath = handlerDir + QStringLiteral("/crashpad_handler.exe");
#endif
#if defined(Q_OS_WIN)
    const base::FilePath handler(handlerPath.toStdWString());
#else
    const base::FilePath handler(handlerPath.toStdString());
#endif

    QDir hDir(handlerDir);
#if defined(Q_OS_MAC)
    if (!hDir.exists(QStringLiteral("crashpad_handler"))) {
#else
    if (!hDir.exists(QStringLiteral("crashpad_handler.exe"))) {
#endif
        LOG_WARN("Crashpad: handler not found at {}", handlerPath.toStdString());
        return false;
    }

    QString dbDir = databaseDir;
    if (dbDir.isEmpty())
        dbDir = handlerDir + QStringLiteral("/Crashpad");
    QDir().mkpath(dbDir);
#if defined(Q_OS_WIN)
    const std::wstring dbDirNative = dbDir.toStdWString();
    const base::FilePath database(dbDirNative);
    const base::FilePath metricsDir(dbDirNative);
#else
    const std::string dbDirNative = dbDir.toStdString();
    const base::FilePath database(dbDirNative);
    const base::FilePath metricsDir(dbDirNative);
#endif

    std::map<std::string, std::string> annotations;
    annotations["prod"] = "ClientForFrame";
    annotations["ver"] = "0.1";

    std::vector<std::string> arguments;

    // CrashpadClient 必须存活至进程结束：若在栈上则函数返回时析构会释放 Mach 异常端口的 send right，
    // 任务仍指向已失效端口，正常退出阶段（或 IMK/mach 消息）可能 SIGSEGV。见 crashpad_client_mac.cc 中
    // SetHandlerMachPort / exception_port_ 成员析构。
    static crashpad::CrashpadClient* s_client = nullptr;
    static bool s_finished = false;
    if (s_finished)
        return s_client != nullptr;
    s_client = new crashpad::CrashpadClient();
    const bool ok = s_client->StartHandler(
        handler,
        database,
        metricsDir,
        std::string(),  // url: 不上传，仅本地 minidump
        annotations,
        arguments,
        false,  // restartable
        false   // asynchronous_start
    );
    s_finished = true;
    if (!ok) {
        delete s_client;
        s_client = nullptr;
    }
    if (ok)
        LOG_INFO("Crashpad initialized: handler={} database={}", handlerPath.toStdString(), dbDir.toStdString());
    else
        LOG_WARN("Crashpad StartHandler failed");
    return ok;
#endif
}

} // namespace Common
