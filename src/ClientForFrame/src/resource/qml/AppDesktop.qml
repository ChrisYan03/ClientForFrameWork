// 客户端框架主桌面：展示可加载的独立组件（App）图标，点击后加载对应组件
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    signal openApp(string appId)

    property int cellSize: 120
    property int spacing: 24

    Flow {
        id: appGrid
        anchors.centerIn: parent
        spacing: root.spacing

        // 图像人脸识别组件
        Item {
            width: root.cellSize + root.spacing
            height: root.cellSize + 32
            Rectangle {
                id: picMatchBg
                width: root.cellSize
                height: root.cellSize
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 12
                color: picMatchMouse.containsMouse && appController && appController.themeColors ? appController.themeColors.appTileBackgroundHover : (appController && appController.themeColors ? appController.themeColors.appTileBackground : "#f1f3f4")
                border.width: picMatchMouse.containsMouse ? 1 : 0
                border.color: appController && appController.themeColors ? appController.themeColors.appTileBorder : "#1a73e8"

                Image {
                    anchors.centerIn: parent
                    source: "qrc:/icons/face_recognition.svg"
                    sourceSize.width: 48
                    sourceSize.height: 48
                }

                MouseArea {
                    id: picMatchMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.openApp("picmatch")
                }
            }
            Label {
                anchors.top: picMatchBg.bottom
                anchors.topMargin: 8
                anchors.horizontalCenter: parent.horizontalCenter
                text: "图像人脸识别"
                font.pixelSize: 13
                font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                color: appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
            }
        }
    }
}
