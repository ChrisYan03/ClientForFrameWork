# Windows 下生成 Crashpad 成果物指南

用于在 Windows 上构建 Crashpad，得到与 Mac 对应的头文件 + 静态库 + `crashpad_handler.exe`，并放入 `ext/lib/crashpad/Win64` 供主工程链接与运行期使用。

## 一、前置条件

- **Visual Studio**：带 C++ 的桌面开发（MSVC），建议 2019 或 2022。
- **Windows SDK**：随 VS 安装即可，建议使用较新版本。
- **Git**：用于拉取 depot_tools 和 Crashpad 源码。
- **Python 3**：depot_tools 会用到。**强烈建议使用 3.10 或 3.11**（与 Chromium 官方测试环境一致）；避免使用 3.12+ 或 3.14，否则会频繁遇到已移除模块（如 `pipes`、`urlparse`、`collections.Mapping` 等）的兼容性问题。安装后可将该版本 Python 放在 PATH 最前，或单独用其路径运行 gclient。

## 二、安装 depot_tools

Crashpad 使用 Chromium 的 GN + Ninja 构建，需先安装 depot_tools（内含 `fetch`、`gn`、`ninja`、`gclient`）：

1. 克隆 depot_tools（路径不要有空格、不要放在 UAC 保护目录）：
   ```bat
   cd C:\workspace
   git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
   ```
2. 将 `C:\workspace\depot_tools` 加入系统或用户 **PATH**，并确保其排在其它 Python/ Git 之前。
3. 打开新的 **cmd.exe**，执行一次：
   ```bat
   gclient
   ```
   首次运行会安装 Windows 依赖（如 Python、msysgit 等）。

## 三、获取 Crashpad 源码

两种方式任选其一。

### 方式 A：使用官方 fetch（推荐，版本清晰）

```bat
mkdir C:\crashpad
cd C:\crashpad
fetch crashpad
```

会在当前目录下生成 `crashpad` 仓库，且自动执行 `gclient sync` 拉齐依赖（含 mini_chromium 等）。

### 方式 B：使用现有 ext 下的源码（与 Mac 一致）

若你已有 `ext/OrginPackage/crashpad-main`（与 Mac 构建同源），可在该目录下直接构建，无需再 fetch。此时“根目录”为 `ext/OrginPackage/crashpad-main`，下面第四、五步中的 `crashpad` 请替换为该路径。

## 四、生成构建目录（GN）

在 Crashpad 源码根目录（`crashpad` 或 `ext/OrginPackage/crashpad-main`）下执行：

```bat
cd C:\crashpad\crashpad
gn gen out/Default --args="is_debug=false target_cpu=\"x64\" extra_cflags=\"/MD\""
```

如需 Debug 可再生成一个目录：

```bat
gn gen out/Debug --args="is_debug=true"
```

可选参数示例（`gn args out/Default` 可编辑）：

- `is_debug=true`：Debug 构建  
- `target_cpu="x64"`：64 位（默认即为 x64）  
- `target_cpu="x86"`：32 位  
- `extra_cflags="/MD"`：**Windows 必须项**，让 Crashpad 静态库与 Qt/主工程使用相同 CRT（`/MD`）。  
  若缺少此项，主工程链接时会出现 `LNK2038 RuntimeLibrary MT_StaticRelease vs MD_DynamicRelease`。

## 五、编译

```bat
ninja -C out/Default
```

构建完成后会得到：

- **可执行文件**：`out/Default/crashpad_handler.exe`
- **静态库**：在 `out/Default/obj/` 下，例如：
  - `out/Default/obj/client/client.lib`
  - `out/Default/obj/client/common.lib`
  - `out/Default/obj/util/util.lib`
  - `out/Default/obj/minidump/minidump.lib`
  - `out/Default/obj/minidump/format.lib`
  - `out/Default/obj/snapshot/snapshot.lib`
  - `out/Default/obj/snapshot/context.lib`
  - `out/Default/obj/third_party/mini_chromium/mini_chromium/base/base.lib`
  - 以及 `compat`、`util/net` 等若被主工程或 handler 依赖则也需要

## 五.五、gclient 已就绪时（当前状态）

若你已在 `ext/OrginPackage/depot_tools/depot_tools` 下用 **Python 3.10** 跑通 `py -3.10 gclient.py`，则：

1. **把 depot_tools 加入 PATH**（便于后面用 `gn`、`ninja`、`gclient`）  
   将 `E:\ClientForFrameWork\ext\OrginPackage\depot_tools\depot_tools` 加入用户或系统 PATH。

2. **二选一**：
   - **方式 A（推荐，需能访问 Google 源）**：新建一个干净目录，用 `fetch crashpad` 拉代码并同步依赖，再在该目录下执行下面的“生成构建目录”和“编译”。
   - **方式 B（沿用现有源码）**：使用 `ext/OrginPackage/crashpad-main` 下的源码时，需在**其父目录**放 `.gclient` 并执行一次 `gclient sync` 拉齐 DEPS（buildtools、mini_chromium 等）；Crashpad 的 DEPS 要求 solution 名为 `crashpad` 且目录名为 `crashpad`，若当前是 `crashpad-main/crashpad-main`，可把内层目录改名为 `crashpad` 并在外层建 `.gclient` 再 sync，或新建 `crashpad_win`、用 `.gclient` + `gclient sync` 得到标准布局后再构建。

3. 在**已 sync 好的 Crashpad 源码根目录**（即含 `BUILD.gn`、`DEPS`、`third_party` 的那一层）执行下面“生成构建目录”和“编译”，并把产物整理到 `ext/lib/crashpad/Win64`。

## 六、整理到 ext/lib/crashpad/Win64

与 Mac 的 `ext/lib/crashpad/Mac` 对应，Windows 成果物建议放到 `ext/lib/crashpad/Win64`，便于 CMake 统一查找。

1. **创建目录**  
   - `ext/lib/crashpad/Win64`  
   - `ext/lib/crashpad/Win64/obj`（若你希望和 Mac 一样把 obj 放在下面）

2. **拷贝 handler**  
   - 将 `out/Default/crashpad_handler.exe` 复制到 `ext/lib/crashpad/Win64/`。

3. **拷贝静态库**  
   将 `out/Default/obj` 下主工程和 handler 依赖的 `.lib` 按相同相对路径拷到 `ext/lib/crashpad/Win64/obj/`，例如：
   - `obj/client/client.lib`
   - `obj/client/common.lib`
   - `obj/util/util.lib`
   - `obj/util/net.lib`（若有）
   - `obj/minidump/minidump.lib`
   - `obj/minidump/format.lib`
   - `obj/snapshot/snapshot.lib`
   - `obj/snapshot/context.lib`
   - `obj/third_party/mini_chromium/mini_chromium/base/base.lib`
   - `obj/compat/compat.lib`（若有）

这样 CMake 里用 `CRASHPAD_OBJ="${CRASHPAD_WIN}/obj"` 即可与 Mac 的 `CRASHPAD_OBJ="${CRASHPAD_MAC}/obj"` 写法一致。

4. **头文件**  
   - 你已有 `ext/include/crashpad`（与 Mac 共用），无需再拷，CMake 继续用 `CRASHPAD_INCLUDE` 即可。

## 七、主工程中已做的适配（简要）

- **CMake**：在 `src/ClientForFrame/CMakeLists.txt` 中已为 `WIN32` 增加 Crashpad 分支：当存在 `ext/lib/crashpad/Win64/obj/client/client.lib` 时，会定义 `HAVE_CRASHPAD`、加入 include、链接上述 `.lib` 并拷贝 `crashpad_handler.exe` 到运行目录。
- **初始化**：`Common::initializeCrashpad(handlerDir, databaseDir)` 在 Windows 下会查找 `crashpad_handler.exe` 并启动 handler，minidump 写入 `databaseDir`（默认 `handlerDir/Crashpad`）。

完成以上步骤后，在 Windows 上编译并运行主程序即可使用 Crashpad 进行崩溃捕获；若未放置 Win64 成果物，CMake 不会开启 Crashpad，行为与未集成时一致。
