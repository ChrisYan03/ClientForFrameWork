import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * PicMatch 工具栏组件
 * 替代原来的QWidget按钮栏（运行/停止/配置）
 */
Rectangle {
    id: root

    // 属性
    property bool running: false                    // 运行状态
    property var themeColors: ({}): ({})           // 主题色

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

    height: 38
    color: bgColor
    border.color: borderColor
    border.width: 0
    border.bottomWidth: 1

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

            ToolTip.text: qsTr("启动")
            ToolTip.visible: hovered
            ToolTip.delay: 500

            Image {
                source: "qrc:/resource/icons/start.svg"
                width: 16
                height: 16
                anchors.centerIn: parent
            }

            background: Rectangle {
                color: runButton.hovered ? root.buttonHover : "transparent"
                radius: 4
            }

            onClicked: root.runClicked()
        }

        // 停止按钮
        ToolButton {
            id: stopButton
            Layout.preferredWidth: 46
            Layout.preferredHeight: 38
            enabled: root.running

            ToolTip.text: qsTr("停止")
            ToolTip.visible: hovered
            ToolTip.delay: 500

            Image {
                source: "qrc:/resource/icons/stop.svg"
                width: 16
                height: 16
                anchors.centerIn: parent
            }

            background: Rectangle {
                color: stopButton.hovered ? root.buttonHover : "transparent"
                radius: 4
            }

            onClicked: root.stopClicked()
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

            ToolTip.text: qsTr("配置")
            ToolTip.visible: hovered
            ToolTip.delay: 500

            Image {
                source: "qrc:/resource/icons/settings.svg"
                width: 16
                height: 16
                anchors.centerIn: parent
            }

            background: Rectangle {
                color: configButton.hovered ? root.buttonHover : "transparent"
                radius: 4
            }

            onClicked: root.configClicked()
        }
    }

    // 运行时运行按钮显示绿色
    RunningIndicator {
        visible: root.running
        anchors.left: runButton.left
        anchors.top: runButton.top
        anchors.margins: 4
    }
}

/**
 * 运行指示器（绿色圆点）
 */
Rectangle {
    width: 8
    height: 8
    radius: 4
    color: "#4CAF50"
}
