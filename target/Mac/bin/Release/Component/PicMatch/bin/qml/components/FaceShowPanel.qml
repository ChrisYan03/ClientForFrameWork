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

    onFaceCountChanged: refreshNonce++

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

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
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
                    color: "#2d2d30"
                    radius: 4

                    property string faceIdValue: root.faceModel
                        ? (faceId || "")
                        : ((modelData && modelData.id) ? modelData.id : "")
                    property string providerUrlValue: root.faceModel
                        ? (imageProviderUrl || "")
                        : ((modelData && modelData.imageProviderUrl) ? modelData.imageProviderUrl : "")
                    property string fileUrlValue: root.faceModel
                        ? (imageFileUrl || "")
                        : ((modelData && modelData.imageFileUrl) ? modelData.imageFileUrl : "")
                    property string dataUrlValue: root.faceModel
                        ? (imageDataUrl || "")
                        : ((modelData && modelData.imageDataUrl) ? modelData.imageDataUrl : "")
                    property real confidenceValue: root.faceModel
                        ? (confidence !== undefined ? confidence : 0)
                        : ((modelData && modelData.attributes && modelData.attributes.confidence !== undefined) ? modelData.attributes.confidence : 0)
                    property string resolvedSource: {
                        var base = providerUrlValue || fileUrlValue || dataUrlValue
                        if (!base || base.length === 0)
                            return ""
                        if (base.indexOf("image://") === 0 || base.indexOf("file://") === 0)
                            return base + (base.indexOf("?") >= 0 ? "&" : "?") + "r=" + root.refreshNonce + "_" + index
                        return base
                    }

                    onResolvedSourceChanged: {
                        console.log("FaceShowPanel resolvedSource changed:",
                                    "faceId=", faceIdValue,
                                    "index=", index,
                                    "source=", resolvedSource)
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 64
                            Layout.preferredHeight: 64
                            color: "#1e1e1e"
                            radius: 4
                            clip: true

                            Image {
                                id: faceImage
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectFit
                                source: faceCard.resolvedSource
                                cache: false
                                asynchronous: false

                                onStatusChanged: {
                                    console.log("FaceShowPanel image status:",
                                                "faceId=", faceCard.faceIdValue,
                                                "index=", index,
                                                "status=", status,
                                                "source=", source)
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Label {
                                text: faceIdValue ? ("Face " + faceIdValue) : "--"
                                color: "#cccccc"
                                font.pixelSize: 12
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Label {
                                text: confidenceValue > 0 ? ((confidenceValue * 100).toFixed(0) + "%") : "--"
                                color: "#888888"
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

        // 空状态
        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
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
