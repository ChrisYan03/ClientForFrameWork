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

        // 动态加载已注册的组件图标
        Repeater {
            model: appController ? appController.loadedComponents : []

            Item {
                width: root.cellSize + root.spacing
                height: root.cellSize + 32

                Rectangle {
                    id: appBg
                    width: root.cellSize
                    height: root.cellSize
                    anchors.horizontalCenter: parent.horizontalCenter
                    radius: 12
                    color: appMouse.containsMouse && appController && appController.themeColors ? appController.themeColors.appTileBackgroundHover : (appController && appController.themeColors ? appController.themeColors.appTileBackground : "#f1f3f4")
                    border.width: appMouse.containsMouse ? 1 : 0
                    border.color: appController && appController.themeColors ? appController.themeColors.appTileBorder : "#1a73e8"

                    Image {
                        anchors.centerIn: parent
                        // 根据组件ID获取对应的图标路径
                        source: {
                            var iconPath = appController ? appController.getComponentIconPath(modelData) : ""
                            if (iconPath) {
                                return iconPath
                            }
                            // 根据组件ID回退到默认图标
                            if (modelData === "PicMatch") {
                                return "qrc:/icons/face_recognition.svg"
                            }
                            return "qrc:/icons/app_default.svg"
                        }
                        sourceSize.width: 48
                        sourceSize.height: 48
                    }

                    MouseArea {
                        id: appMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.openApp(modelData)
                    }
                }

                Label {
                    anchors.top: appBg.bottom
                    anchors.topMargin: 8
                    anchors.horizontalCenter: parent.horizontalCenter
                    // 优先使用从 manifest 读取的名称，如果没有则使用组件 ID
                    text: {
                        var name = appController ? appController.getComponentName(modelData) : ""
                        return name ? name : modelData
                    }
                    font.pixelSize: 13
                    font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                    color: appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
                }
            }
        }
    }
}
