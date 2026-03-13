# PicMatch QML 链路问题修复总结

日期：2026-03-13  
适用平台：Windows  
范围：本次修改仅涉及 Windows 下的 QML/宿主窗口链路（`PlayerHostItem` 为主）

## 背景

Windows 下 PicMatch 组件存在两个现象（本次修复均以 Windows 运行表现为准）：

1. 图1：从主框架进入组件后，红框区域的人脸图标未消失。  
2. 图2：运行后停止，红框区域未恢复到初始状态。

本次排查以 Windows 运行日志为依据，优先验证 QML 流程，不以代码猜测直接修复。

## 结论概览

- 图2根因已确认并修复（Windows）：停播时宿主窗口需要显式隐藏并触发刷新。  
- 图1根因已确认并修复（Windows）：组件切入时（`running=false`）宿主窗口被提前显示，覆盖了预期界面状态。  
- 最终修复点集中在 Windows 的 `PlayerHostItem` 可见性控制，未引入定时延迟类“假修复”。

## 图2问题（停止后未回初始状态）

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

## 图1问题（切入组件后图标未消失）

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

