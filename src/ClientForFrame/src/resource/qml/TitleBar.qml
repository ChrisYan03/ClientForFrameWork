import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Rectangle {
    id: titleBarRoot
    z: 100
    color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.titleBarBackground : "#ffffff"
    border.width: 0
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.titleBarBorder : "#e0e0e0"
    }

    signal requestMove(real dx, real dy)
    signal requestMaximize()
    signal backToDesktopClicked()
    signal settingsClicked()

    property bool pressed: false
    property real lastMouseX: 0
    property real lastMouseY: 0
    property string statusText: ""
    property bool isMaximized: false
    /** 是否显示“返回桌面”按钮（当前已加载独立组件时为 true） */
    property bool showBackButton: false

    // 独立小窗口显示 ToolTip，浅色系
    Window {
        id: tooltipWindow
        flags: Qt.Tool | Qt.FramelessWindowHint
        color: "transparent"
        visible: false
        width: tipContent.implicitWidth + 24
        height: tipContent.implicitHeight + 16
        minimumWidth: 60
        minimumHeight: 28
        Rectangle {
            anchors.fill: parent
            color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.tooltipBackground : "#ffffff"
            border.color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.tooltipBorder : "#dadce0"
            radius: 4
            Label {
                id: tipContent
                anchors.centerIn: parent
                color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.tooltipText : "#3c4043"
                font.pixelSize: 12
                text: tooltipWindow.tipText
            }
        }
        property string tipText: ""
    }

        RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 0
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        spacing: 8

        // 左侧：图标 + 标题（紧贴一体）
        Item {
            id: leftBlock
            Layout.preferredWidth: leftRow.implicitWidth
            Layout.preferredHeight: 38
            Layout.alignment: Qt.AlignVCenter

            RowLayout {
                id: leftRow
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4
                Image {
                    source: "qrc:/icons/app_title.svg"
                    sourceSize.width: 20
                    sourceSize.height: 20
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                    Layout.alignment: Qt.AlignVCenter
                }
                Label {
                    text: typeof appController !== "undefined" && appController ? appController.pageTitle : "小闫客户端"
                    font.pixelSize: 13
                    font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                    color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
                    Layout.alignment: Qt.AlignVCenter
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onPressed: function(mouse) {
                    titleBarRoot.pressed = true
                    titleBarRoot.lastMouseX = mouse.globalPosition.x
                    titleBarRoot.lastMouseY = mouse.globalPosition.y
                }
                onReleased: {
                    titleBarRoot.pressed = false
                }
                onPositionChanged: function(mouse) {
                    if (titleBarRoot.pressed) {
                        var dx = mouse.globalPosition.x - titleBarRoot.lastMouseX
                        var dy = mouse.globalPosition.y - titleBarRoot.lastMouseY
                        titleBarRoot.lastMouseX = mouse.globalPosition.x
                        titleBarRoot.lastMouseY = mouse.globalPosition.y
                        titleBarRoot.requestMove(dx, dy)
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 8
        }

        Label {
            text: titleBarRoot.statusText
            font.pixelSize: 12
            font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
            color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textSecondary : "#5f6368"
            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: 12
            visible: text.length > 0 && (typeof appController !== "undefined" && appController && appController.hasRunnableComponent)
        }

        RowLayout {
            spacing: 0
            Layout.alignment: Qt.AlignVCenter

            // 设置：主框架按钮，始终显示
            Button {
                id: settingsBtn
                visible: true
                implicitWidth: 46
                implicitHeight: 38
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                contentItem.opacity: settingsBtn.hovered ? 1 : 0.9
                background: Rectangle {
                    anchors.fill: parent
                    color: settingsBtn.hovered && appController && appController.themeColors ? appController.themeColors.buttonHover : "transparent"
                    radius: 0
                }
                icon.source: "qrc:/icons/settings.svg"
                icon.width: 16
                icon.height: 16
                icon.color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
                display: AbstractButton.IconOnly
                hoverEnabled: true
                onHoveredChanged: {
                    if (hovered) {
                        tooltipWindow.tipText = "设置"
                        var pt = settingsBtn.mapToGlobal(0, settingsBtn.height + 4)
                        tooltipWindow.x = pt.x
                        tooltipWindow.y = pt.y
                        tooltipWindow.visible = true
                    } else if (!backBtn.hovered && !maxBtn.hovered && !closeBtn.hovered)
                        tooltipWindow.visible = false
                }
                onClicked: titleBarRoot.settingsClicked()
            }

            // 返回主界面：组件页时展示
            Button {
                id: backBtn
                visible: titleBarRoot.showBackButton
                implicitWidth: 46
                implicitHeight: 38
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                contentItem.opacity: backBtn.hovered ? 1 : 0.9
                background: Rectangle {
                    anchors.fill: parent
                    color: backBtn.hovered && appController && appController.themeColors ? appController.themeColors.buttonHover : "transparent"
                    radius: 0
                }
                icon.source: "qrc:/icons/home.svg"
                icon.width: 16
                icon.height: 16
                icon.color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
                display: AbstractButton.IconOnly
                hoverEnabled: true
                onHoveredChanged: {
                    if (hovered) {
                        tooltipWindow.tipText = "返回主界面"
                        var pt = backBtn.mapToGlobal(0, backBtn.height + 4)
                        tooltipWindow.x = pt.x
                        tooltipWindow.y = pt.y
                        tooltipWindow.visible = true
                    } else if (!settingsBtn.hovered && !maxBtn.hovered && !closeBtn.hovered)
                        tooltipWindow.visible = false
                }
                onClicked: titleBarRoot.backToDesktopClicked()
            }

            Button {
                id: maxBtn
                implicitWidth: 46
                implicitHeight: 38
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                contentItem.opacity: maxBtn.hovered ? 1 : 0.9
                background: Rectangle {
                    anchors.fill: parent
                    color: maxBtn.hovered && appController && appController.themeColors ? appController.themeColors.buttonHover : "transparent"
                    radius: 0
                }
                icon.source: titleBarRoot.isMaximized ? "qrc:/icons/restore.svg" : "qrc:/icons/maximize.svg"
                icon.width: 16
                icon.height: 16
                icon.color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
                display: AbstractButton.IconOnly
                hoverEnabled: true
                onHoveredChanged: {
                    if (hovered) {
                        tooltipWindow.tipText = titleBarRoot.isMaximized ? "还原" : "最大化"
                        var pt = maxBtn.mapToGlobal(0, maxBtn.height + 4)
                        tooltipWindow.x = pt.x
                        tooltipWindow.y = pt.y
                        tooltipWindow.visible = true
                    } else if (!settingsBtn.hovered && !backBtn.hovered && !closeBtn.hovered)
                        tooltipWindow.visible = false
                }
                onClicked: requestMaximize()
            }
            Button {
                id: closeBtn
                implicitWidth: 46
                implicitHeight: 38
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                contentItem.opacity: closeBtn.hovered ? 1 : 0.9
                background: Rectangle {
                    anchors.fill: parent
                    color: closeBtn.hovered ? "#e81123" : "transparent"
                    radius: 0
                }
                icon.source: "qrc:/icons/close.svg"
                icon.width: 16
                icon.height: 16
                icon.color: closeBtn.hovered ? "#ffffff" : (typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232")
                display: AbstractButton.IconOnly
                hoverEnabled: true
                onHoveredChanged: {
                    if (hovered) {
                        tooltipWindow.tipText = "关闭应用程序"
                        var pt = closeBtn.mapToGlobal(0, closeBtn.height + 4)
                        tooltipWindow.x = pt.x
                        tooltipWindow.y = pt.y
                        tooltipWindow.visible = true
                    } else if (!settingsBtn.hovered && !backBtn.hovered && !maxBtn.hovered)
                        tooltipWindow.visible = false
                }
                onClicked: {
                    tooltipWindow.visible = false
                    if (appController)
                        appController.closeApp()
                }
            }
        }
    }
}
