# 从 Qt Widgets 迁移到 QML 指南

本文档说明如何将当前 **ClientForFrame**（Qt Widgets）项目迁移到 **QML/Qt Quick** 界面。

## 一、整体思路

| 原技术栈 | 迁移后 |
|---------|--------|
| `QApplication` + `QWidget` | `QGuiApplication` + `QQmlApplicationEngine` 加载 `.qml` |
| `TitleWidget` / `ClientMainWidget` 等 C++ 控件 | QML 文件（如 `MainWindow.qml`、`TitleBar.qml`） |
| 业务逻辑（PicPlayer、PicRecognition） | 保留在 C++，通过 **Q_PROPERTY / Q_INVOKABLE / 信号** 暴露给 QML |
| 无边框窗口 + 标题栏拖动 | QML `Window` + `flags: Qt.FramelessWindowHint` + `MouseArea` 拖动 |

## 二、迁移步骤概览

1. **CMake**：增加 Qt Quick（及可选 QuickControls2），并加入 QML 文件与 C++ 桥接类。
2. **入口**：`main.cpp` 改为使用 `QGuiApplication` + `QQmlApplicationEngine`，加载 `main.qml`。
3. **界面**：用 QML 重做主窗口、标题栏、内容区（先占位，再逐步替换）。
4. **桥接**：用 C++ 类（如 `AppController`）暴露开始/停止/关闭、状态等，供 QML 调用。
5. **特殊**：PicPlayer 依赖 `winId()` 嵌入原生窗口，需在 QML 中通过 C++ 提供的“占位项”或 `QQuickItem` 嵌入窗口，见下文。

## 三、PicPlayer 嵌入说明（重要）

当前 `PicMatchWidget` 使用 `PicPlayer_RegisterWindow(m_handle, m_playerWidget->winId())` 把播放器画到 Qt Widgets 的某个控件上。迁移到 QML 后有两种常见做法：

- **方案 A（推荐）**：在 C++ 里实现一个继承自 `QQuickItem` 的类（例如 `PlayerHostItem`），在该 Item 的 `geometryChanged` 里创建/维护一个子 `QWindow`，把子窗口的 `winId()` 传给 `PicPlayer_RegisterWindow`，并在 QML 里把这个 Item 放在内容区对应位置（通过 `setParentItem` 或 QML 里引用该 Item）。这样 QML 只负责布局，真正嵌入由 C++ 完成。
- **方案 B**：主界面仍用 Qt Widgets（如 `QMainWindow`），仅部分界面用 `QQuickWidget` 嵌入 QML。这样改动较小，但整体不是“纯 QML”应用。

本仓库已提供 **方案 A** 的骨架：C++ 端保留 PicPlayer/PicRecognition 调用，通过 `AppController` 与 QML 交互；QML 端主窗口、标题栏已用 QML 实现，**播放器区域** 先使用占位（如一个 Rectangle），你只需后续实现 `PlayerHostItem` 并把其 `winId()` 传给现有 PicPlayer 初始化逻辑即可。

## 四、如何启用 QML 构建

在配置 CMake 时打开 `USE_QML` 选项即可用 QML 作为主界面：

```bash
cd build
cmake -DUSE_QML=ON ..
cmake --build .
```

- `USE_QML=ON`：使用 QML 界面入口（`main_qml.cpp` + `qrc:/qml/main.qml`），无边框窗口与标题栏为 QML 实现。
- `USE_QML=OFF`（默认）：使用原有 Qt Widgets 入口（`main.cpp` + `ClientMainWidget`）。

## 五、已提供的文件与修改

- **CMakeLists.txt**  
  - 增加 `find_package(Qt6 COMPONENTS Quick QuickControls2)`（或 Qt5 对应组件）。  
  - 将 `main.qml`、`MainWindow.qml`、`TitleBar.qml` 等加入 `qt_add_resources` 或 `set(QML_FILES ...)` 并随资源一起部署。  
  - 把桥接类（如 `AppController`）和（若实现）`PlayerHostItem` 加入目标，并 `target_link_libraries(... Quick QuickControls2)`。

- **main.cpp**  
  - 使用 `QGuiApplication`。  
  - 创建 `QQmlApplicationEngine`，注册 `AppController` 或通过 `rootContext()->setContextProperty("appController", ...)` 注入。  
  - `engine->load(QUrl("qrc:/main.qml")` 或等效路径。

- **QML 界面**  
  - `main.qml`：仅加载主窗口（如 `MainWindow {}`）。  
  - `MainWindow.qml`：无边框 `Window`，内嵌自定义标题栏 + 内容区。  
  - `TitleBar.qml`：标题、状态、开始/停止/关闭按钮，通过 `appController.start()` / `stop()` / `close()` 等与 C++ 交互。

- **C++ 桥接**  
  - `AppController`（或你命名的类）：  
    - `Q_INVOKABLE void start()` / `stop()` / `close()`；  
    - `Q_PROPERTY(QString statusText ...)` 供标题栏显示状态；  
    - 内部调用现有 `PicMatchWidget` 或抽出的“业务层”的 Run/Quit。

按上述步骤后，应用将以 QML 为主界面启动，业务逻辑仍在 C++，后续可逐步把更多 UI 从 Widgets 迁到 QML，并完成 PicPlayer 的窗口嵌入实现。

## 六、后续可做

- 用 QML 重做“人脸展示区域”（如用 `ListView` + 自定义 delegate 显示图片）。  
- 将 `StyleManager` 的 theme 以 QML 的 `Qt.labs.settings` 或 C++ 属性形式暴露，实现主题切换。  
- 把 PicPlayer 的 `PlayerHostItem` 实现完整，并在 `AppController` 中完成创建与 `winId()` 的传递。
