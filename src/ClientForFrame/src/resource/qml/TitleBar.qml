import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Rectangle {
    id: titleBarRoot
    z: 100
    color: "#f3f3f3"
    border.width: 0
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: "#e4e4e4"
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
        anchors.leftMargin: 10
        anchors.rightMargin: 6
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        spacing: 6

        // 左侧：图标 + 标题（VS Code 风格紧凑）
        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 80
            Layout.preferredHeight: 35

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
                    font.pixelSize: 12
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
            font.pixelSize: 11
            color: "#5f6368"
            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: 8
        }

        RowLayout {
            spacing: 2
            Layout.alignment: Qt.AlignVCenter

            Button {
                id: startBtn
                implicitWidth: 28
                implicitHeight: 28
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                background: Rectangle {
                    color: parent.hovered ? "#4CAF50" : "transparent"
                    radius: 2
                }
                icon.source: "qrc:/icons/start.svg"
                icon.width: 14
                icon.height: 14
                icon.color: "transparent"
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
                implicitWidth: 28
                implicitHeight: 28
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                background: Rectangle {
                    color: parent.hovered ? "#e0e0e0" : "transparent"
                    radius: 2
                }
                icon.source: "qrc:/icons/stop.svg"
                icon.width: 14
                icon.height: 14
                icon.color: "transparent"
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
                implicitWidth: 28
                implicitHeight: 28
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                background: Rectangle {
                    color: parent.hovered ? "#e0e0e0" : "transparent"
                    radius: 2
                }
                contentItem: Text {
                    text: titleBarRoot.isMaximized ? "❐" : "□"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: "#3c4043"
                }
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
                implicitWidth: 28
                implicitHeight: 28
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                background: Rectangle {
                    color: parent.hovered ? "#c42b1c" : "transparent"
                    radius: 2
                }
                icon.source: "qrc:/icons/close.svg"
                icon.width: 14
                icon.height: 14
                icon.color: "transparent"
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
