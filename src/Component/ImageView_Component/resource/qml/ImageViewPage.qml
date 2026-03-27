import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ImageViewCore 1.0

Item {
    id: root
    width: StackView.view ? StackView.view.width : 0
    height: StackView.view ? StackView.view.height : 0

    Rectangle {
        anchors.fill: parent
        color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.contentBackground : "#f7f7f7"
    }

    ImageViewHostItem {
        id: playerHostItem
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        // Windows 原生子窗口会覆盖同层 QML，预留顶部区域用于显示悬浮按钮。
        anchors.topMargin: 64
    }

    Button {
        id: floatingBtn
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 12
        anchors.rightMargin: 12
        implicitWidth: 46
        implicitHeight: 38
        topPadding: 0
        bottomPadding: 0
        leftPadding: 0
        rightPadding: 0
        hoverEnabled: true
        display: AbstractButton.TextOnly
        text: playerHostItem.running ? qsTr("暂停") : qsTr("启动")
        contentItem: Label {
            text: floatingBtn.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 13
            font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
            color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
            opacity: floatingBtn.hovered ? 1 : 0.9
        }
        background: Rectangle {
            anchors.fill: parent
            color: floatingBtn.hovered && appController && appController.themeColors ? appController.themeColors.buttonHover : "transparent"
            radius: 0
        }
        onClicked: playerHostItem.toggle()
   }

    Component.onCompleted: {
        // 进入组件不自动启动，等待用户点击“启动”按钮。
    }

    Component.onDestruction: {
        playerHostItem.stop()
    }

    onVisibleChanged: {
        if (!visible) {
            playerHostItem.stop()
        }
    }
}
