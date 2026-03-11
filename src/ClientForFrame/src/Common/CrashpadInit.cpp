// Copyright 2025 ClientForFrameWork. Crashpad 初始化实现。

#include "CrashpadInit.h"
#include "LogUtil.h"
#include <QDir>
#include <QStandardPaths>

#if (defined(Q_OS_MAC) || defined(Q_OS_WIN)) && defined(HAVE_CRASHPAD)
#include "client/crashpad_client.h"
#include "base/files/file_path.h"
#include <string>
#include <map>
#include <vector>
#endif

namespace Common {

bool initializeCrashpad(const QString& handlerDir, const QString& databaseDir)
{
#if !defined(HAVE_CRASHPAD)
    Q_UNUSED(handlerDir);
    Q_UNUSED(databaseDir);
    return false;
#elif (defined(Q_OS_MAC) || defined(Q_OS_WIN)) && defined(HAVE_CRASHPAD)

    std::string handlerPath;
#if defined(Q_OS_MAC)
    handlerPath = (handlerDir + QStringLiteral("/crashpad_handler")).toStdString();
#else
    handlerPath = (handlerDir + QStringLiteral("/crashpad_handler.exe")).toStdString();
#endif
    const base::FilePath handler(handlerPath);

    QDir hDir(handlerDir);
#if defined(Q_OS_MAC)
    if (!hDir.exists(QStringLiteral("crashpad_handler"))) {
#else
    if (!hDir.exists(QStringLiteral("crashpad_handler.exe"))) {
#endif
        LOG_WARN("Crashpad: handler not found at {}", handlerPath);
        return false;
    }

    QString dbDir = databaseDir;
    if (dbDir.isEmpty())
        dbDir = handlerDir + QStringLiteral("/Crashpad");
    QDir().mkpath(dbDir);
    const base::FilePath database(dbDir.toStdString());
    const base::FilePath metricsDir(dbDir.toStdString());

    std::map<std::string, std::string> annotations;
    annotations["prod"] = "ClientForFrame";
    annotations["ver"] = "0.1";

    std::vector<std::string> arguments;

    crashpad::CrashpadClient client;
    const bool ok = client.StartHandler(
        handler,
        database,
        metricsDir,
        std::string(),  // url: 不上传，仅本地 minidump
        annotations,
        arguments,
        false,  // restartable
        false   // asynchronous_start
    );
    if (ok)
        LOG_INFO("Crashpad initialized: handler={} database={}", handlerPath, dbDir.toStdString());
    else
        LOG_WARN("Crashpad StartHandler failed");
    return ok;

#else
    Q_UNUSED(handlerDir);
    Q_UNUSED(databaseDir);
    return false;
#endif
}

} // namespace Common
