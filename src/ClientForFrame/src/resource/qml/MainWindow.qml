import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import App 1.0

Window {
    id: root
    visible: true
    title: "图像识别系统"
    width: 1400
    height: 900
    minimumWidth: 1400
    minimumHeight: 900
    flags: Qt.Window | Qt.FramelessWindowHint
    color: "transparent"

    property string mainStatusText: "● 就绪"
    property bool isMaximized: visibility === Window.Maximized

    // 圆角背景容器（窗口透明，仅此矩形区域不透明）；最大化时取消圆角
    Rectangle {
        id: roundBack
        anchors.fill: parent
        radius: root.isMaximized ? 0 : 10
        color: "#f3f3f3"
        clip: true

        TitleBar {
            id: titleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 35
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

        PlayerHostItem {
            objectName: "playerHost"
            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
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
