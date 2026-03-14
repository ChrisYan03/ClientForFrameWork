import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic

/**
 * PicMatch 人脸显示面板组件
 */
Rectangle {
    id: root

    property var faceModel: null
    property var faceList: []
    property var themeColors: ({})
    property int refreshNonce: 0

    signal faceClicked(string faceId)

    color: themeColors.contentBackground || "#1e1e1e"

    property int faceCount: root.faceModel ? (root.faceModel.count || 0) : (root.faceList ? root.faceList.length : 0)

    onFaceCountChanged: {
        refreshNonce++
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // 标题
        Label {
            text: qsTr("检测到的人脸")
            color: themeColors.textPrimary || "#cccccc"
            font.pixelSize: 13
            font.family: "'SF Pro Text', 'Helvetica Neue', 'Segoe UI', 'Microsoft YaHei UI', sans-serif"
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollView {
                id: faceScroll
                anchors.fill: parent
                clip: true

                ListView {
                    id: faceListView
                    anchors.fill: parent
                    clip: true
                    spacing: 8
                    model: root.faceModel || root.faceList
                    interactive: true
                    delegate: Rectangle {
                    id: faceCard
                    width: faceListView.width
                    height: 80
                    color: root.themeColors.appTileBackground || "#2d2d30"
                    radius: 4

                    property string faceIdValue: root.faceModel
                        ? (faceId || "")
                        : ((modelData && modelData.id) ? modelData.id : "")
                    property string providerUrlValue: root.faceModel
                        ? (imageProviderUrl || "")
                        : ((modelData && modelData.imageProviderUrl) ? modelData.imageProviderUrl : "")
                    property real confidenceValue: root.faceModel
                        ? (confidence !== undefined ? confidence : 0)
                        : ((modelData && modelData.attributes && modelData.attributes.confidence !== undefined) ? modelData.attributes.confidence : 0)
                    property string resolvedSource: providerUrlValue

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 64
                            Layout.preferredHeight: 64
                            color: root.themeColors.contentBackground || "#1e1e1e"
                            radius: 4
                            clip: true

                            Image {
                                id: faceImage
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectFit
                                source: faceCard.resolvedSource
                                cache: false
                                asynchronous: true
                                mipmap: true

                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: faceIdValue ? ("Face " + faceIdValue) : "--"
                                color: root.themeColors.textPrimary || "#cccccc"
                                font.pixelSize: 12
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Label {
                                text: confidenceValue > 0 ? ((confidenceValue * 100).toFixed(0) + "%") : "--"
                                color: root.themeColors.textSecondary || "#888888"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (faceIdValue)
                                root.faceClicked(faceIdValue)
                        }
                    }
                    }
                }
            }

            // 空状态覆盖层（不参与 ColumnLayout 分配，避免挤压 ListView 高度）
            Label {
                anchors.fill: parent
                visible: root.faceCount === 0
                text: qsTr("未检测到人脸")
                color: themeColors.textPrimary || "#cccccc"
                opacity: 0.5
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

}
