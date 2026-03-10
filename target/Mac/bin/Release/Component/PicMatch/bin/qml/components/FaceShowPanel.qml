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

    property int faceCount: {
        if (root.faceModel)
            return root.faceModel.count || 0
        if (root.faceList)
            return root.faceList.length || 0
        return 0
    }

    onFaceCountChanged: {
        refreshNonce++
        if (faceListView.forceLayout)
            faceListView.forceLayout()
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
                    width: faceListView.width
                    height: 96
                    color: "#2d2d30"
                    radius: 4

                    property string faceIdValue: root.faceModel
                        ? (faceId || "")
                        : ((modelData && modelData.id) ? modelData.id : "")
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
                        var base = fileUrlValue || dataUrlValue
                        if (!base || base.length === 0)
                            return ""
                        if (base.indexOf("file://") === 0)
                            return base + "?r=" + root.refreshNonce + "_" + index
                        return base
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 8

                        Rectangle {
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 80
                            color: "#1e1e1e"
                            radius: 2
                            clip: true

                            Image {
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectFit
                                source: parent.parent.parent.resolvedSource
                                cache: false
                                asynchronous: false
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Label {
                                text: faceIdValue ? ("Face " + faceIdValue) : "--"
                                color: "#cccccc"
                                font.pixelSize: 11
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            Label {
                                text: confidenceValue > 0 ? ((confidenceValue * 100).toFixed(0) + "%") : "--"
                                color: "#888888"
                                font.pixelSize: 10
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
