# PicMatch QML 链路问题修复总结

日期：2026-03-13  
适用平台：Windows / macOS  
范围：本次修改分为两部分：  
- Windows：QML 宿主窗口链路可见性控制（`PlayerHostItem` 为主，已在下文「Windows」章节说明）  
- macOS：在保持 Windows 行为不变的前提下，针对红色人脸展示区域「只在缩放窗口后才刷新」的问题做专门适配（`PlayerHostItem` + QML）

## 背景

Windows 下 PicMatch 组件存在两个现象（本次修复均以 Windows 运行表现为准）：

1. 图1：从主框架进入组件后，红框区域的人脸图标未消失。  
2. 图2：运行后停止，红框区域未恢复到初始状态。

本次排查以 Windows 运行日志为依据，优先验证 QML 流程，不以代码猜测直接修复。

## 结论概览

- 图2根因已确认并修复（Windows）：停播时宿主窗口需要显式隐藏并触发刷新。  
- 图1根因已确认并修复（Windows）：组件切入时（`running=false`）宿主窗口被提前显示，覆盖了预期界面状态。  
- 最终修复点集中在 Windows 的 `PlayerHostItem` 可见性控制，未引入定时延迟类“假修复”。

## Windows 图2问题（停止后未回初始状态）

### 根因

停播信号链路虽然正确到达 `PlayerHostItem`，但需要在 `running=false` 时明确执行宿主窗口隐藏与界面刷新，避免最后一帧残留视觉。

### 修改点（Windows QML 宿主链路）

文件：`src/Component/PicMatch_Component/src/QmlBridge/PlayerHostItem.cpp`

- 在 `onViewModelRunningChanged(false)` 分支中执行（Windows）：
  - `m_hostWindow->hide()`
  - `QQuickWindow::update()` / `requestUpdate()`
  - Windows 下 `RedrawWindow(...)` 强制重绘

### 验证结论

日志显示停播后 `hostVisibleAfter:false`，并有对应刷新路径，图2问题确认修复。

## Windows 图1问题（切入组件后图标未消失）

### 根因

运行时证据显示：在组件初次切入、`viewModel.running=false` 时，`PlayerHostItem` 的宿主窗口已被创建并显示（`hostVisible:true`）。  
这导致红框区域出现不符合预期的前景覆盖（表现为图标/残留不消失）。

### 修改点（Windows QML 宿主链路）

文件：`src/Component/PicMatch_Component/src/QmlBridge/PlayerHostItem.cpp`

1. `ensureHostWindowCreated()`  
   - 从“创建后无条件 `show()`”调整为：
   - `running=true` 才 `show()`，`running=false` 保持 `hide()`。

2. `updateHostWindowGeometry()`  
   - 从“几何更新时不可见就 `show()`”调整为按运行态同步：
   - `running=true` 才显示；`running=false` 则保持/切回隐藏。

### 验证结论

修复后再次复现，问题不再出现，图1确认修复。

## 涉及文件（本次 Windows 有效修复）

- `src/Component/PicMatch_Component/src/QmlBridge/PlayerHostItem.cpp`

> 说明：排查过程中对其他链路做过临时打点验证；最终保留的功能修复聚焦在 Windows 的上述 QML 宿主文件。

## Windows 回归建议

1. 桌面进入 PicMatch（不点击运行）确认红框区域初始显示正确。  
2. 点击运行 -> 停止，确认红框区域恢复初始状态。  
3. 在窗口缩放、最大化/还原后重复上述流程，确认无回归。

---

## macOS：人脸展示区不随内容刷新（仅缩放窗口后才更新）

### 现象与结论概览

macOS 下，主播放区域的人脸框实时更新正常，但右侧「人脸展示区」存在以下现象：

- 人脸检测、模型数据、`Image` 资源加载都已经完成，但右侧 QML 区域长期停留在旧帧；
- 只有在用户手动缩放/最大化/还原主窗口后，右侧区域才一次性刷出正确的人脸列表。

通过跨多轮日志验证，可以得到两个关键结论：

1. C++ / QML 数据链路在 macOS 上是通的：  
   - `PicMatchViewModel::onImageUpdated` 能正常生成 `FaceData` 并写入 `FaceListModel`；  
   - QML `ListView` 的 `count`、delegate、`Image` 尺寸/状态都在变化；
2. 真正「卡住」的是 QML 顶层 `QQuickWindow` 的渲染：  
   - 在人脸数量变化后，长时间看不到对应的 `frameSwapped`；  
   - 一旦窗口发生 `Expose`（用户缩放、最大化/还原），就会立刻触发一批 `frameSwapped`，画面随之恢复正常。

最终结论：**macOS 下问题根因在于 QQuickWindow 在该组合场景中不会主动产生新帧，必须“刺激”一次 Expose 才会刷新。**

### macOS 修复点（仅修改 Mac 分支，不影响 Windows）

文件：`src/Component/PicMatch_Component/src/QmlBridge/PlayerHostItem.cpp`

1. 监听 `PicMatchViewModel::imageUpdated`，统一在 C++ 侧触发 QQuickWindow 刷新  

   ```cpp
   m_imageUpdatedConn = QObject::connect(
       typedVm, &PicMatchViewModel::imageUpdated, this,
       [this](const QString& showId, const QString&) {
           Q_UNUSED(showId);
           if (QQuickWindow* quickWin = window()) {
#if defined(Q_OS_MAC)
               // 仅 macOS：延迟到下一轮事件循环，并执行事件处理 + 极小尺寸变化
               QTimer::singleShot(0, this, [this]() {
                   if (QQuickWindow* w = window()) {
                       w->update();
                       w->requestUpdate();
                       QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                       const int ww = w->width(), wh = w->height();
                       if (ww > 0 && wh > 0) {
                           w->resize(ww + 1, wh);
                           QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                           w->resize(ww, wh);
                           w->requestUpdate();
                       }
                   }
               });
#else
               // 其他平台（包括 Windows）保持简单的 update/requestUpdate 行为
               quickWin->update();
               quickWin->requestUpdate();
#endif
           }
       },
       Qt::QueuedConnection);
   ```

   说明：
   - **Windows 路径完全保持原有行为**（简单的 `update()/requestUpdate()`），不受本次 macOS 适配影响；
   - macOS 路径额外做三件事：  
     1. `QTimer::singleShot(0, ...)`：把刷新请求推迟到下一轮事件循环，避免在当前槽里被 Qt 忽略；  
     2. `QCoreApplication::processEvents(...)`：在本次逻辑内主动处理一次事件队列，让 pending 的 `UpdateRequest` 有机会被消费；  
     3. 对 `QQuickWindow` 做一次极小的宽度抖动（`width → width+1 → width`），相当于程序模拟了一次用户缩放，从而触发 `Expose` 并强制产生一帧。

2. 其他平台不做任何逻辑改动  

   - 所有 macOS 特有逻辑均放在 `#if defined(Q_OS_MAC)` 宏内；  
   - Windows 下仍沿用前文「Windows 修复」中描述的宿主窗口可见性与重绘策略。

### QML 层的调整（FaceShowPanel / PicMatchPage）

文件：  
- `src/Component/PicMatch_Component/resource/qml/components/FaceShowPanel.qml`  
- `src/Component/PicMatch_Component/resource/qml/PicMatchPage.qml`

为保证后续维护简洁，本次 macOS 适配结束后对 QML 做了如下收尾：

- 清理了为排查问题临时增加的大量 `agentDebugLog` / `logSceneBounds` / `onFrameSwapped` 等调试打点；  
- 恢复 `FaceShowPanel` 为一个常规的 `ListView + delegate + Image` 结构（仅保留必要的 `faceCount` / `refreshNonce` 逻辑），不依赖任何额外的 `requestUpdate()` 调用。

这部分变更**不改变业务行为**，只是去掉调试期遗留的噪声代码。

### macOS 回归建议

1. 在 macOS 上从主框架进入 PicMatch 组件，不运行时确认：  
   - 左侧主播放区域显示正常；  
   - 右侧人脸展示区为空状态文案正确显示。  
2. 点击运行后，观察多张图片轮播：  
   - 每次检测到新的人脸，右侧人脸展示区都能在**不缩放窗口**的情况下及时刷新；  
   - 不再出现「只有缩放窗口后才看到最新人脸」的情况。  
3. 在窗口缩放、最大化/还原、多屏切换后重复步骤 1/2，确认无闪烁或回退到旧行为。  

