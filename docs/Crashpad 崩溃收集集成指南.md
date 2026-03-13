# Crashpad 崩溃收集集成指南

本文档说明如何在 ClientForFrameWork 中引入 [Google Crashpad](https://chromium.googlesource.com/crashpad/crashpad)，
用于在 macOS 与 Windows 上捕获崩溃并生成 minidump，便于后续分析与上报。

## 一、前置准备

### 1. 安装 depot_tools（用于拉取与构建 Crashpad）

```bash
# 克隆 depot_tools
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git ~/depot_tools
# 加入 PATH（建议写入 ~/.zshrc 或 ~/.bashrc）
export PATH="$HOME/depot_tools:$PATH"
```

### 2. 获取 Crashpad 源码

```bash
mkdir -p ~/crashpad
cd ~/crashpad
fetch crashpad
```

### 3. 构建 Crashpad

Crashpad 使用 GN + Ninja 构建，不直接使用 CMake。

**macOS（含 arm64）：**

```bash
cd ~/crashpad/crashpad
gn gen out/Default
# 若需 Debug：在 out/Default/args.gn 中设置 is_debug = true
ninja -C out/Default
```

**Windows（x64）：**

```bash
cd ~/crashpad/crashpad
gn gen out/Default --args="target_cpu=\"x64\""
ninja -C out/Default
```

构建完成后，在 `out/Default` 下会得到：

- **crashpad_handler**（macOS/Linux）或 **crashpad_handler.exe**（Windows）—— 必须与主程序同目录或指定路径
- 静态库（路径以你本机 `ninja -C out/Default` 输出为准）：
  - macOS：`obj/client/libclient.a`、`obj/util/libutil.a`、`obj/common/libcommon.a`、`obj/third_party/mini_chromium/mini_chromium/base/libbase.a`、`obj/util/libmig_output.a`
  - Windows：对应目录下的 `.lib` 文件

### 4. 将 Crashpad 产物放入工程

建议在工程内统一管理第三方库（与现有 `ext` 一致）：

- **方式 A：复制构建产物到 ext**

  - 在 `ext` 下创建目录，例如：  
    `ext/crashpad/`（或 `ext/OrginPackage/crashpad/`）
  - 将 `~/crashpad/crashpad` 的**源码**复制或链接到 `ext/crashpad/crashpad_src`（或按你现有命名）
  - 将对应平台的 `out/Default` 下的 **crashpad_handler**（或 crashpad_handler.exe）以及需要的静态库复制到例如：
    - `ext/crashpad/lib/Mac/`（macOS）
    - `ext/crashpad/lib/Win64/`（Windows）

- **方式 B：在 CMake 中直接引用本机 Crashpad 构建目录**

  - 在 CMake 里通过 `CRASHPAD_ROOT` 或类似变量指向 `~/crashpad/crashpad`，  
    然后 `include_directories(...)`、`link_directories(...)` 并链接所需库。

无论哪种方式，都需要在 CMake 中：

- 添加 Crashpad 的 **include** 路径（含 `client`、`third_party/mini_chromium` 等）
- 链接 **client、util、common、base** 等库（macOS 还需 **mig_output**）
- 把 **crashpad_handler** 拷贝到可执行文件所在目录（或通过代码指定其路径）

## 二、工程内集成步骤

### 1. CMake 选项与依赖

在顶层或 ClientForFrame 的 `CMakeLists.txt` 中增加可选开关，例如：

```cmake
option(USE_CRASHPAD "Enable Crashpad crash reporting" OFF)

if(USE_CRASHPAD)
  set(CRASHPAD_ROOT "${CMAKE_SOURCE_DIR}/../ext/crashpad/crashpad_src" CACHE PATH "Crashpad source root")
  # 或 set(CRASHPAD_ROOT "$ENV{HOME}/crashpad/crashpad")
  set(CRASHPAD_BUILD "${CRASHPAD_ROOT}/out/Default")
  include_directories(
    ${CRASHPAD_ROOT}
    ${CRASHPAD_ROOT}/third_party/mini_chromium/mini_chromium
    ${CRASHPAD_BUILD}/gen
  )
  if(APPLE)
    link_directories(${CRASHPAD_BUILD})
    target_link_libraries(ClientForFrame PRIVATE
      client base common util mig_output
      "-framework Security" "-framework AppKit"
    )
  elseif(WIN32)
    target_link_libraries(ClientForFrame PRIVATE
      ${CRASHPAD_BUILD}/obj/client/client.lib
      ${CRASHPAD_BUILD}/obj/util/util.lib
      # ... 其他库路径以你实际 GN 输出为准
    )
  endif()
  target_compile_definitions(ClientForFrame PRIVATE USE_CRASHPAD=1)
  # 将 crashpad_handler 复制到可执行文件目录
  add_custom_command(TARGET ClientForFrame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      $<IF:$<PLATFORM_ID:Windows>,${CRASHPAD_BUILD}/crashpad_handler.exe,${CRASHPAD_BUILD}/crashpad_handler>
      $<TARGET_FILE_DIR:ClientForFrame>
  )
endif()
```

具体库名、路径需根据你本机 `gn gen` / `ninja` 的实际输出调整（Windows 下多为 `.lib`，macOS 下多为 `.a`）。

### 2. 主程序入口处初始化（越早越好）

在 `main()` 中，**在创建 `QApplication` 或任何可能崩溃的代码之前** 调用 Crashpad 初始化，这样 Qt 初始化阶段的崩溃也能被捕获。例如在 `main_qml.cpp` 中：

```cpp
int main(int argc, char *argv[])
{
#if defined(USE_CRASHPAD)
  CrashpadInit::initialize(argc, argv);
#endif

#if defined(Q_OS_WIN)
  qputenv("QSG_RHI_BACKEND", "opengl");
#endif
  QApplication app(argc, argv);
  // ...
}
```

`CrashpadInit::initialize(argc, argv)` 内部会：

- 根据 `argv[0]`（以及 Windows 上必要时用 `GetModuleFileName`）解析可执行文件所在目录
- 指定 **crashpad_handler** 的路径（例如与 exe 同目录）
- 指定 **database** / **metrics** 目录（例如 `exe_dir/crashpad_reports`）
- 可选：设置 **upload URL**（若暂时只写本地 minidump，可传空字符串）
- 设置 **annotations**（如 product、version）便于后续区分版本

### 3. 崩溃报告存放与上传

- **仅本地：**  
  `CrashpadInit::initialize` 中 `url` 传空；minidump 会写在 `database` 目录下，可用 Breakpad 的 `minidump_stackwalk` 或 Visual Studio / Xcode 等工具分析。

- **上报到服务器：**  
  需要自建或使用第三方支持 Breakpad/Crashpad 格式的接收端（例如 [Sentry](https://sentry.io)、[BugSplat](https://bugsplat.com)、自建 minidump 接收服务）。  
  在 `CrashpadInit::initialize` 里将 `url` 设为该服务的 minidump 提交地址即可。

## 三、注意事项

1. **crashpad_handler 必须与主程序架构一致**（例如都是 arm64 或都是 x64），且需随应用一起分发（同目录或通过配置指定路径）。
2. **C++ 标准**：Crashpad 需要 C++14 或更高，你当前工程已是 C++14，满足要求。
3. **macOS**：链接时可能需要 `-framework Security`、`-framework AppKit` 等，见上 CMake 示例。
4. **首次可先不设 upload URL**，只打开 `USE_CRASHPAD` 并确认崩溃时能在本地生成 minidump，再配置上传。

## 四、参考

- [Crashpad - Developing Crashpad](https://chromium.googlesource.com/crashpad/crashpad/+/HEAD/doc/developing.md)
- [Integrating Crashpad with MacOS Qt application](https://stackoverflow.com/questions/61346222/integrating-crashpad-with-macos-qt-application)
- [How to Build Google Crashpad | BugSplat](https://docs.bugsplat.com/introduction/getting-started/integrations/cross-platform/crashpad/how-to-build-google-crashpad)

按上述步骤即可在 ClientForFrameWork 中引入 Crashpad；若你希望，我可以再根据你当前的 `CMakeLists.txt` 和 `main_qml.cpp` 写出一份可直接粘贴的补丁（含 `CrashpadInit` 的占位实现与 CMake 片段）。
