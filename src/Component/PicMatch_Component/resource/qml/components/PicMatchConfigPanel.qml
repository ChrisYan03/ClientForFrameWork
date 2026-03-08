import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * PicMatch 配置面板组件
 * 替代原来的QWidget配置面板
 */
Rectangle {
    id: root

    // 属性
    property alias dataPath: pathEdit.text           // 数据路径
    property var themeColors: ({}): ({})           // 主题色

    // 信号
    signal applyClicked()
    signal browseClicked()
    signal backClicked()

    // 内部颜色计算
    property color contentBg: themeColors.contentBackground || "#1e1e1e"
    property color textPrimary: themeColors.textPrimary || "#cccccc"
    property color borderColor: themeColors.border || "#3c3c3c"
    property color buttonPrimary: themeColors.appTileBorder || "#007acc"
    property color buttonHover: themeColors.buttonHover || "#3c3c3c"

    color: contentBg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // 标题
        Label {
            text: qsTr("图片查看路径")
            color: textPrimary
            font.pixelSize: 13
            font.family: "'SF Pro Text', 'Helvetica Neue', 'Segoe UI', 'Microsoft YaHei UI', sans-serif"
        }

        // 路径输入框
        RowLayout {
            spacing: 8

            TextField {
                id: pathEdit
                Layout.fillWidth: true
                placeholderText: qsTr("留空则使用默认路径")
                minimumHeight: 28

                // 样式
                background: Rectangle {
                    color: borderColor
                    radius: 4
                    border.color: borderColor
                }
                color: textPrimary
            }

            Button {
                text: qsTr("浏览...")
                Layout.preferredHeight: 28
                padding.left: 12
                padding.right: 12

                background: Rectangle {
                    color: borderColor
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: root.browseClicked()
            }
        }

        // 空白填充
        Item {
            Layout.fillHeight: true
        }

        // 按钮行
        RowLayout {
            spacing: 8

            Item {
                Layout.fillWidth: true
            }

            // 应用/确定按钮
            Button {
                text: qsTr("确定")
                Layout.preferredHeight: 28
                padding.left: 16
                padding.right: 16

                background: Rectangle {
                    color: buttonPrimary
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: root.applyClicked()
            }

            // 返回按钮
            Button {
                text: qsTr("返回")
                Layout.preferredHeight: 28
                padding.left: 16
                padding.right: 16

                background: Rectangle {
                    color: borderColor
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: textPrimary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: root.backClicked()
            }
        }
    }
}
