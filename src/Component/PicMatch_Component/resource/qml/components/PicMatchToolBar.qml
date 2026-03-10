import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

/**
 * PicMatch 工具栏组件
 * 替代原来的QWidget按钮栏（运行/停止/配置）
 */
Rectangle {
    id: root

    // 属性
    property bool running: false                    // 运行状态
    property var themeColors: ({})                 // 主题色

    // 信号
    signal runClicked()
    signal stopClicked()
    signal configClicked()

    // 内部颜色计算
    property color bgColor: themeColors.titleBarBackground || "#2d2d30"
    property color borderColor: themeColors.titleBarBorder || "#3c3c3c"
    property color buttonHover: themeColors.buttonHover || "#3c3c3c"
    property color textColor: themeColors.textPrimary || "#cccccc"
    property color primaryColor: themeColors.primaryColor || "#007acc"
    property color successColor: "#4CAF50"  // 运行时的绿色

    function showTip(btn, text) {
        tooltipWindow.tipText = text
        var pt = btn.mapToGlobal(0, btn.height + 4)
        tooltipWindow.x = pt.x
        tooltipWindow.y = pt.y
        tooltipWindow.visible = true
    }

    function tryHideTip() {
        if (!runButton.hovered && !stopButton.hovered && !configButton.hovered)
            tooltipWindow.visible = false
    }

    height: 38
    color: bgColor
    border.color: borderColor
    border.width: 0

    // 仅底边线（Rectangle 无 border.bottomWidth，用子元素实现）
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: root.borderColor
    }

    // 组件按钮提示：参考框架标题栏样式
    Window {
        id: tooltipWindow
        flags: Qt.Tool | Qt.FramelessWindowHint
        color: "transparent"
        visible: false
        width: tipContent.implicitWidth + 24
        height: tipContent.implicitHeight + 16
        minimumWidth: 60
        minimumHeight: 28
        property string tipText: ""

        Rectangle {
            anchors.fill: parent
            color: themeColors.tooltipBackground || "#ffffff"
            border.color: themeColors.tooltipBorder || "#dadce0"
            radius: 4

            Label {
                id: tipContent
                anchors.centerIn: parent
                color: themeColors.tooltipText || "#3c4043"
                font.pixelSize: 12
                text: tooltipWindow.tipText
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 0
        spacing: 8

        // 运行按钮
        ToolButton {
            id: runButton
            Layout.preferredWidth: 46
            Layout.preferredHeight: 38
            enabled: !root.running
            hoverEnabled: true

            Image {
                source: Qt.resolvedUrl("../../resource/icons/start.svg")
                width: 16
                height: 16
                anchors.centerIn: parent
            }

            background: Rectangle {
                color: runButton.hovered ? root.buttonHover : "transparent"
                radius: 4
            }

            // 运行时绿色圆点（放在按钮内才能锚定）
            Rectangle {
                visible: root.running
                width: 8
                height: 8
                radius: 4
                color: "#4CAF50"
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 4
            }

            onClicked: root.runClicked()
            onHoveredChanged: {
                if (hovered)
                    root.showTip(runButton, qsTr("启动"))
                else
                    root.tryHideTip()
            }
        }

        // 停止按钮
        ToolButton {
            id: stopButton
            Layout.preferredWidth: 46
            Layout.preferredHeight: 38
            enabled: root.running
            hoverEnabled: true

            Image {
                source: Qt.resolvedUrl("../../resource/icons/stop.svg")
                width: 16
                height: 16
                anchors.centerIn: parent
            }

            background: Rectangle {
                color: stopButton.hovered ? root.buttonHover : "transparent"
                radius: 4
            }

            onClicked: root.stopClicked()
            onHoveredChanged: {
                if (hovered)
                    root.showTip(stopButton, qsTr("停止"))
                else
                    root.tryHideTip()
            }
        }

        // 填充
        Item {
            Layout.fillWidth: true
        }

        // 配置按钮
        ToolButton {
            id: configButton
            Layout.preferredWidth: 46
            Layout.preferredHeight: 38
            hoverEnabled: true

            Image {
                source: Qt.resolvedUrl("../../resource/icons/settings.svg")
                width: 16
                height: 16
                anchors.centerIn: parent
            }

            background: Rectangle {
                color: configButton.hovered ? root.buttonHover : "transparent"
                radius: 4
            }

            onClicked: root.configClicked()
            onHoveredChanged: {
                if (hovered)
                    root.showTip(configButton, qsTr("配置"))
                else
                    root.tryHideTip()
            }
        }
    }
}
