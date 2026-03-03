// 主窗口：布局与样式跨平台（Windows / macOS 共用）
// - 圆角：Win 下由 DWM 或内容区绘制，Mac 下由 setMask 圆角
// - 字体：QSS/TitleBar 使用 Segoe UI + SF Pro Text/Helvetica Neue 回退，Mac 上自动用系统字体
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import App 1.0

Window {
    id: root
    // 由 C++ 在首帧就绪后再 show，避免 Windows 下白屏
    visible: false
    title: "图像识别系统"
    width: 1400
    height: 900
    minimumWidth: 1400
    minimumHeight: 900
    flags: Qt.Window | Qt.FramelessWindowHint
    // Windows 下透明会导致整窗白屏；与内容区一致，VS Code 风格浅灰
    color: "#f3f3f3"

    property string mainStatusText: "● 就绪"
    property bool isMaximized: visibility === Window.Maximized
    readonly property int titleBarHeight: 38
    readonly property int cornerRadius: 8
    readonly property real splitRatio: 0.7
    readonly property int contentMargin: 0
    readonly property int splitterWidth: 1
    readonly property color splitterColor: "#e4e4e4"

    // 圆角内容区（VS Code 风格：8px 圆角 + 细边框）；最大化时取消圆角
    Rectangle {
        id: roundBack
        anchors.fill: parent
        radius: root.isMaximized ? 0 : root.cornerRadius
        color: "#ffffff"
        border.width: root.isMaximized ? 0 : 1
        border.color: "#e0e0e0"
        clip: true

        TitleBar {
            id: titleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: root.titleBarHeight
            statusText: root.mainStatusText
            isMaximized: root.isMaximized
            onRequestMove: (dx, dy) => {
                if (!root.isMaximized)
                    root.x += dx; root.y += dy
            }
            onRequestMaximize: {
                if (root.visibility === Window.Maximized)
                    root.showNormal()
                else
                    root.showMaximized()
            }
            onRunClicked: {
                root.mainStatusText = "● 运行中"
            }
            onStopClicked: {
                root.mainStatusText = "● 已停止"
            }
        }

        Item {
            id: contentArea
            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: root.contentMargin
            anchors.rightMargin: root.contentMargin
            anchors.bottomMargin: root.contentMargin

            PlayerHostItem {
                objectName: "playerHost"
                anchors.fill: parent
            }

            Rectangle {
                id: splitterLine
                width: root.splitterWidth
                anchors.top: contentArea.top
                anchors.bottom: contentArea.bottom
                x: Math.round(contentArea.width * root.splitRatio) - Math.floor(root.splitterWidth / 2)
                color: splitterHover.hovered ? "#cccccc" : root.splitterColor
                z: 10
                visible: contentArea.width > 0 && contentArea.height > 0
                MouseArea {
                    id: splitterHover
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.SizeHorCursor
                }
            }
        }
    }

    Component.onCompleted: {
        if (appController)
            root.mainStatusText = appController.statusText
        if (root.visibility !== Window.Maximized) {
            var screen = Qt.application.screens.length > 0 ? Qt.application.screens[0] : null
            if (screen) {
                root.x = (screen.width - root.width) / 2
                root.y = (screen.height - root.height) / 2
            }
        }
    }
    Connections {
        target: appController
        function onStatusTextChanged() {
            if (appController)
                root.mainStatusText = appController.statusText
        }
    }
}
