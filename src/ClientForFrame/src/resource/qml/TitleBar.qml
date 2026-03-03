import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Rectangle {
    id: titleBarRoot
    z: 100
    color: "#ffffff"
    border.width: 0
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: "#e0e0e0"
    }

    signal requestMove(real dx, real dy)
    signal requestMaximize()
    signal runClicked()
    signal stopClicked()

    property bool pressed: false
    property real lastMouseX: 0
    property real lastMouseY: 0
    property string statusText: "● 就绪"
    property bool isMaximized: false

    // 独立小窗口显示 ToolTip，浅色系
    Window {
        id: tooltipWindow
        flags: Qt.Tool | Qt.FramelessWindowHint
        color: "transparent"
        visible: false
        width: tipContent.implicitWidth + 24
        height: tipContent.implicitHeight + 16
        minimumWidth: 60
        minimumHeight: 28
        Rectangle {
            anchors.fill: parent
            color: "#ffffff"
            border.color: "#dadce0"
            radius: 4
            Label {
                id: tipContent
                anchors.centerIn: parent
                color: "#3c4043"
                font.pixelSize: 12
                text: tooltipWindow.tipText
            }
        }
        property string tipText: ""
    }

        RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 0
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        spacing: 8

        // 左侧：图标 + 标题（VS Code 风格）
        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 80
            Layout.preferredHeight: 38

            RowLayout {
                anchors.fill: parent
                spacing: 6
                Image {
                    source: "qrc:/icons/app_title.svg"
                    sourceSize.width: 20
                    sourceSize.height: 20
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                    Layout.alignment: Qt.AlignVCenter
                }
                Label {
                    text: "图像识别系统"
                    font.pixelSize: 13
                    font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                    color: "#323232"
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onPressed: function(mouse) {
                    titleBarRoot.pressed = true
                    titleBarRoot.lastMouseX = mouse.globalPosition.x
                    titleBarRoot.lastMouseY = mouse.globalPosition.y
                }
                onReleased: {
                    titleBarRoot.pressed = false
                }
                onPositionChanged: function(mouse) {
                    if (titleBarRoot.pressed) {
                        var dx = mouse.globalPosition.x - titleBarRoot.lastMouseX
                        var dy = mouse.globalPosition.y - titleBarRoot.lastMouseY
                        titleBarRoot.lastMouseX = mouse.globalPosition.x
                        titleBarRoot.lastMouseY = mouse.globalPosition.y
                        titleBarRoot.requestMove(dx, dy)
                    }
                }
            }
        }

        Label {
            text: titleBarRoot.statusText
            font.pixelSize: 12
            font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
            color: "#5f6368"
            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: 12
        }

        RowLayout {
            spacing: 0
            Layout.alignment: Qt.AlignVCenter

            Button {
                id: startBtn
                implicitWidth: 46
                implicitHeight: 38
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                contentItem.opacity: startBtn.hovered ? 1 : 0.9
                background: Rectangle {
                    anchors.fill: parent
                    color: startBtn.hovered ? "#4CAF50" : "transparent"
                    radius: 0
                }
                icon.source: "qrc:/icons/start.svg"
                icon.width: 16
                icon.height: 16
                icon.color: startBtn.hovered ? "#ffffff" : "#323232"
                display: AbstractButton.IconOnly
                hoverEnabled: true
                onHoveredChanged: {
                    if (hovered) {
                        tooltipWindow.tipText = "开始执行图像匹配任务"
                        var pt = startBtn.mapToGlobal(0, startBtn.height + 4)
                        tooltipWindow.x = pt.x
                        tooltipWindow.y = pt.y
                        tooltipWindow.visible = true
                    } else if (!stopBtn.hovered && !maxBtn.hovered && !closeBtn.hovered)
                        tooltipWindow.visible = false
                }
                onClicked: {
                    runClicked()
                    if (appController) appController.start()
                }
            }
            Button {
                id: stopBtn
                implicitWidth: 46
                implicitHeight: 38
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                contentItem.opacity: stopBtn.hovered ? 1 : 0.9
                background: Rectangle {
                    anchors.fill: parent
                    color: stopBtn.hovered ? "#e5e5e5" : "transparent"
                    radius: 0
                }
                icon.source: "qrc:/icons/stop.svg"
                icon.width: 16
                icon.height: 16
                icon.color: "#323232"
                display: AbstractButton.IconOnly
                hoverEnabled: true
                onHoveredChanged: {
                    if (hovered) {
                        tooltipWindow.tipText = "停止当前图像匹配任务"
                        var pt = stopBtn.mapToGlobal(0, stopBtn.height + 4)
                        tooltipWindow.x = pt.x
                        tooltipWindow.y = pt.y
                        tooltipWindow.visible = true
                    } else if (!startBtn.hovered && !maxBtn.hovered && !closeBtn.hovered)
                        tooltipWindow.visible = false
                }
                onClicked: {
                    stopClicked()
                    if (appController) appController.stop()
                }
            }
            Button {
                id: maxBtn
                implicitWidth: 46
                implicitHeight: 38
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                contentItem.opacity: maxBtn.hovered ? 1 : 0.9
                background: Rectangle {
                    anchors.fill: parent
                    color: maxBtn.hovered ? "#e5e5e5" : "transparent"
                    radius: 0
                }
                icon.source: titleBarRoot.isMaximized ? "qrc:/icons/restore.svg" : "qrc:/icons/maximize.svg"
                icon.width: 16
                icon.height: 16
                icon.color: "#323232"
                display: AbstractButton.IconOnly
                hoverEnabled: true
                onHoveredChanged: {
                    if (hovered) {
                        tooltipWindow.tipText = titleBarRoot.isMaximized ? "还原" : "最大化"
                        var pt = maxBtn.mapToGlobal(0, maxBtn.height + 4)
                        tooltipWindow.x = pt.x
                        tooltipWindow.y = pt.y
                        tooltipWindow.visible = true
                    } else if (!startBtn.hovered && !stopBtn.hovered && !closeBtn.hovered)
                        tooltipWindow.visible = false
                }
                onClicked: requestMaximize()
            }
            Button {
                id: closeBtn
                implicitWidth: 46
                implicitHeight: 38
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                contentItem.opacity: closeBtn.hovered ? 1 : 0.9
                background: Rectangle {
                    anchors.fill: parent
                    color: closeBtn.hovered ? "#e81123" : "transparent"
                    radius: 0
                }
                icon.source: "qrc:/icons/close.svg"
                icon.width: 16
                icon.height: 16
                icon.color: closeBtn.hovered ? "#ffffff" : "#323232"
                display: AbstractButton.IconOnly
                hoverEnabled: true
                onHoveredChanged: {
                    if (hovered) {
                        tooltipWindow.tipText = "关闭应用程序"
                        var pt = closeBtn.mapToGlobal(0, closeBtn.height + 4)
                        tooltipWindow.x = pt.x
                        tooltipWindow.y = pt.y
                        tooltipWindow.visible = true
                    } else if (!startBtn.hovered && !stopBtn.hovered && !maxBtn.hovered)
                        tooltipWindow.visible = false
                }
                onClicked: {
                    tooltipWindow.visible = false
                    if (appController)
                        appController.closeApp()
                }
            }
        }
    }
}
