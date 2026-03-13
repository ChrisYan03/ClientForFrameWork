import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import App 1.0
import PicMatchCore 1.0
import "components" 1.0

/**
 * PicMatch 主页面 (MVVM架构)
 * 使用ViewModel + QML组件
 */
Item {
    id: root
    objectName: "picMatchPage"

    // 生命周期管理
    Component.onCompleted: {
        if (typeof appController !== "undefined" && appController) {
            appController.registerComponentHost(playerHost)
            // 初始化ViewModel
            if (picMatchViewModel) {
                picMatchViewModel.initialize()
            }
        }
    }

    Component.onDestruction: {
        if (typeof appController !== "undefined" && appController) {
            appController.unregisterComponentHost()
        }
        // 关闭ViewModel
        if (picMatchViewModel) {
            picMatchViewModel.shutdown()
        }
    }

    // ViewModel - 由C++注册
    PicMatchViewModel {
        id: picMatchViewModel

        // ViewModel 内部已处理图片输入与播放，无需再调 PlayerHostItem
        onImageUpdated: function(showId, imagePath) { }
    }

    // 布局：左侧渲染区域 + 右侧面板
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧：渲染区域（纯 QWindow 宿主，无 QWidget 嵌入）
        PlayerHostItem {
            id: playerHost
            objectName: "playerHost"
            Layout.fillWidth: true
            Layout.fillHeight: true
            viewModel: picMatchViewModel

            // 窗口准备好后，把宿主窗口交给 ViewModel（run 时再注册到 PicPlayer）
            onWidgetReady: {
                if (picMatchViewModel && playerHost.hostWindow) {
                    picMatchViewModel.registerWindow(playerHost.hostWindow)
                }
            }
        }

        // 右侧：控制面板
        Rectangle {
            id: rightPanel
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            color: appController && appController.themeColors ?
                   appController.themeColors.contentBackground : "#1e1e1e"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 工具栏（运行/停止/配置）
                PicMatchToolBar {
                    id: toolBar
                    Layout.fillWidth: true

                    running: picMatchViewModel.running
                    themeColors: appController ? appController.themeColors : {}

                    onRunClicked: picMatchViewModel.run()
                    onStopClicked: picMatchViewModel.stop()
                    onConfigClicked: rightStack.currentIndex = 1
                }

                // 堆栈视图（人脸显示 / 配置面板）
                StackLayout {
                    id: rightStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: 0  // 默认显示人脸面板

                    // 页面0：人脸显示
                    FaceShowPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        faceModel: picMatchViewModel.faceModel
                        themeColors: appController ? appController.themeColors : {}
                        onFaceClicked: function(faceId) {
                            console.log("Face clicked:", faceId)
                            // TODO: 处理人脸点击事件
                        }
                    }

                    // 页面1：配置面板
                    PicMatchConfigPanel {
                        id: configPanel
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        dataPath: picMatchViewModel.dataPath
                        themeColors: appController ? appController.themeColors : {}

                        onApplyClicked: {
                            // 保存路径到ViewModel
                            picMatchViewModel.setDataPath(dataPath)
                            picMatchViewModel.saveConfig()
                            rightStack.currentIndex = 0
                        }

                        onBrowseClicked: {
                            // 打开文件夹选择对话框
                            folderDialog.open()
                        }

                        onBackClicked: {
                            rightStack.currentIndex = 0
                        }
                    }
                }
            }
        }
    }

    // 接收主题变化
    Connections {
        target: appController
        function onThemeColorsChanged() {
            if (picMatchViewModel) {
                picMatchViewModel.applyTheme(appController.themeColors)
            }
        }
    }

    // 文件夹选择对话框
    FolderDialog {
        id: folderDialog
        title: qsTr("选择图片目录")
        onAccepted: {
            // 设置选中的路径
            configPanel.dataPath = selectedFolder
        }
    }
}
