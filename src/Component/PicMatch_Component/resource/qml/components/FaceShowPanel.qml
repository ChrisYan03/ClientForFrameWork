import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic

/**
 * PicMatch 人脸显示面板组件
 * 替代原来的QWidget FaceShowWidget
 */
Rectangle {
    id: root

    // 属性
    property var faceList: ([]) : []          // 人脸列表数据
    property var themeColors: ({}): ({})    // 主题色

    // 信号
    signal faceClicked(string faceId)

    // 内部颜色计算
    property color contentBg: themeColors.contentBackground || "#1e1e1e"
    property color textPrimary: themeColors.textPrimary || "#cccccc"

    color: contentBg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 0

        // 标题
        Label {
            text: qsTr("检测到的人脸")
            color: textPrimary
            font.pixelSize: 13
            font.family: "'SF Pro Text', 'Helvetica Neue', 'Segoe UI', 'Microsoft YaHei UI', sans-serif"
            padding.bottom: 8
        }

        // 人脸列表（使用ListView）
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            GridView {
                id: faceGridView
                model: root.faceList
                cellWidth: 80
                cellHeight: 100
                interactive: false

                delegate: FaceCard {
                    faceData: modelData
                    onClicked: root.faceClicked(faceData.id)
                }
            }
        }

        // 空状态提示
        Label {
            visible: root.faceList.length === 0
            text: qsTr("未检测到人脸")
            color: textPrimary
            opacity: 0.5
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.centerIn: parent
        }
    }
}

/**
 * 人脸卡片组件
 */
Item {
    id: faceCardRoot

    property var faceData: ({}): ({})
    signal clicked(string faceId)

    width: 70
    height: 90

    Rectangle {
        id: cardBg
        anchors.fill: parent
        color: "#2d2d30"
        radius: 4

        // 人脸图片区域（使用占位符，实际应该显示faceData中的图片）
        Rectangle {
            id: faceImageArea
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "#1e1e1e"
            radius: 4

            // 临时占位图标
            Image {
                source: "qrc:/resource/icons/face_recognition.svg"
                width: 24
                height: 24
                anchors.centerIn: parent
                opacity: 0.5
            }
        }

        // 人脸信息
        Column {
            anchors.top: faceImageArea.bottom
            anchors.topMargin: 4
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 2

            Label {
                text: "Face " + (faceCardRoot.faceData.id || "")
                color: "#cccccc"
                font.pixelSize: 10
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }

            // 置信度
            Label {
                text: (faceCardRoot.faceData.attributes ?
                       (faceCardRoot.faceData.attributes.confidence * 100).toFixed(0) + "%" : "--")
                color: "#888888"
                font.pixelSize: 9
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            if (faceCardRoot.faceData && faceCardRoot.faceData.id) {
                faceCardRoot.clicked(faceCardRoot.faceData.id)
            }
        }
    }
}
