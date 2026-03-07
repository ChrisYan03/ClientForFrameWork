#include "CrashpadInit.h"

#if defined(USE_CRASHPAD)

#include "client/crashpad_client.h"
#include "base/files/file_path.h"
#include <string>
#include <vector>
#include <map>

#if defined(Q_OS_WIN) || defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

#if defined(__APPLE__) || defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

namespace CrashpadInit {

namespace {

#if defined(Q_OS_WIN) || defined(_WIN32) || defined(WIN32)
std::wstring getExecutableDirW()
{
    wchar_t buf[MAX_PATH + 1] = {};
    if (::GetModuleFileNameW(nullptr, buf, MAX_PATH) == 0)
        return {};
    std::wstring path(buf);
    std::wstring::size_type last = path.find_last_of(L"/\\");
    if (last != std::wstring::npos)
        path.resize(last);
    else
        path.clear();
    return path;
}
#else
std::string getExecutableDir(int argc, char *argv[])
{
    (void)argc;
    std::string exePath;
    if (argv[0] && argv[0][0] == '/') {
        char resolved[PATH_MAX] = {};
        if (realpath(argv[0], resolved))
            exePath = resolved;
        else
            exePath = argv[0];
    } else if (argv[0]) {
        char cwd[PATH_MAX] = {};
        if (getcwd(cwd, sizeof(cwd)))
            exePath = std::string(cwd) + "/" + argv[0];
        else
            exePath = argv[0];
    }
    std::string::size_type last = exePath.find_last_of('/');
    if (last != std::string::npos)
        exePath.resize(last);
    else
        exePath = ".";
    return exePath;
}
#endif

} // namespace

void initialize(int argc, char *argv[])
{
    using namespace crashpad;

#if defined(Q_OS_WIN) || defined(_WIN32) || defined(WIN32)
    std::wstring exeDirW = getExecutableDirW();
    if (exeDirW.empty())
        return;
    base::FilePath exeDir(exeDirW);
    base::FilePath handler = exeDir.Append(FILE_PATH_LITERAL("crashpad_handler.exe"));
    base::FilePath database = exeDir.Append(FILE_PATH_LITERAL("crashpad_reports"));
    base::FilePath metrics = exeDir.Append(FILE_PATH_LITERAL("crashpad_metrics"));
#else
    std::string exeDirStr = getExecutableDir(argc, argv);
    if (exeDirStr.empty())
        return;
    base::FilePath exeDir(exeDirStr);
    base::FilePath handler = exeDir.Append("crashpad_handler");
    base::FilePath database = exeDir.Append("crashpad_reports");
    base::FilePath metrics = exeDir.Append("crashpad_metrics");
#endif

    // 仅本地保存 minidump 时可留空；若需上报则设为你的服务器 URL
    std::string url; // e.g. "https://your-server.com/crash"

    std::map<std::string, std::string> annotations;
    annotations["product"] = "ClientForFrame";
    annotations["version"] = "0.1";

    std::vector<std::string> arguments;

    CrashpadClient client;
    if (!client.StartHandler(handler, database, metrics, url, annotations, arguments, true, false)) {
        // 启动失败可记日志，此处不依赖 Qt/日志库
        return;
    }
}

} // namespace CrashpadInit

#else // !USE_CRASHPAD

namespace CrashpadInit {

void initialize(int /* argc */, char ** /* argv */)
{
}

} // namespace CrashpadInit

#endif
