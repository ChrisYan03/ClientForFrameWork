import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent

    readonly property bool isDark: typeof appController !== "undefined" && appController && appController.theme === 1
    readonly property color trackOff: isDark ? "#3c3c3c" : "#e0e0e0"
    readonly property color trackOn: "#007acc"
    readonly property color thumbColor: "#ffffff"

    Rectangle {
        anchors.fill: parent
        color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.contentBackground : "#ffffff"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 24
        anchors.bottomMargin: 24
        spacing: 14

        Label {
            text: "设置"
            font.pixelSize: 17
            font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
            color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 10
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.border : "#e0e0e0"
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 10
            spacing: 10
            Label {
                text: root.isDark ? "皮肤模式 [深色模式]" : "皮肤模式 [浅色模式]"
                font.pixelSize: 13
                font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
                Layout.alignment: Qt.AlignVCenter
            }
            Item { Layout.preferredWidth: 10 }
            Item {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 22
                Layout.alignment: Qt.AlignVCenter

                property bool checked: root.isDark

                Rectangle {
                    id: track
                    anchors.fill: parent
                    radius: height / 2
                    color: parent.checked ? root.trackOn : root.trackOff
                    border.width: 0
                    Behavior on color { ColorAnimation { duration: 120 } }
                }
                Rectangle {
                    id: thumb
                    width: 18
                    height: 18
                    radius: width / 2
                    y: (parent.height - height) / 2
                    x: parent.checked ? (parent.width - width - 2) : 2
                    color: root.thumbColor
                    Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (appController)
                            appController.setTheme(parent.checked ? 0 : 1)
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
