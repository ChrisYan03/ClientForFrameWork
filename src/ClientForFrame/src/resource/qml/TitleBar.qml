import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Window

Rectangle {
    id: titleBarRoot
    z: 100
    // 动态高度：单组件38px，多组件78px（38+40）
    height: showTabBar ? (titleBarHeight + tabBarHeight) : titleBarHeight
    color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.titleBarBackground : "#ffffff"
    border.width: 0

    // 翻译文本
    property string appTitle: qsTr("小闫客户端")
    property string settingsTooltip: qsTr("设置")
    property string backTooltip: ""
    property string restoreTooltip: qsTr("还原")
    property string maximizeTooltip: qsTr("最大化")
    property string closeTooltip: qsTr("关闭应用程序")
    property string closeTabTooltip: qsTr("关闭标签页")

    // 双层布局相关属性
    readonly property int titleBarHeight: 38
    readonly property int tabBarHeight: 40
    // 显示标签栏的条件：多组件时始终显示（这样主页按钮始终可见）
    property bool showTabBar: typeof appController !== "undefined" && appController && appController.componentCount > 1
    // 当前是否在主界面
    property bool isOnDesktop: true

    // 监听语言变化
    Connections {
        target: typeof appController !== "undefined" ? appController : null
        function onCurrentLanguageChanged() {
            appTitle = qsTr("小闫客户端")
            settingsTooltip = qsTr("设置")
            backTooltip = ""
            restoreTooltip = qsTr("还原")
            maximizeTooltip = qsTr("最大化")
            closeTooltip = qsTr("关闭应用程序")
            closeTabTooltip = qsTr("关闭标签页")
        }
    }

    signal requestMove(real dx, real dy)
    signal requestMaximize()
    signal backToDesktopClicked()
    signal switchToDesktop()
    signal settingsClicked()
    signal tabClicked(string appId)
    signal tabCloseClicked(string appId)

    property bool pressed: false
    property real lastMouseX: 0
    property real lastMouseY: 0
    property string statusText: ""
    property bool isMaximized: false
    property bool showBackButton: false
    property bool showSettingsButton: true

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

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 第一层：标题栏（38px）
        Rectangle {
            id: firstLayer
            Layout.fillWidth: true
            Layout.preferredHeight: titleBarHeight
            color: "transparent"

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
                            text: typeof appController !== "undefined" && appController ? appController.pageTitle : titleBarRoot.appTitle
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

                    // 设置按钮：单组件模式时切换显示，多组件模式时常驻
                    Button {
                        id: settingsBtn
                        visible: !titleBarRoot.showTabBar ? titleBarRoot.showSettingsButton : true
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
                                tooltipWindow.tipText = titleBarRoot.settingsTooltip
                                var pt = settingsBtn.mapToGlobal(0, settingsBtn.height + 4)
                                tooltipWindow.x = pt.x
                                tooltipWindow.y = pt.y
                                tooltipWindow.visible = true
                            } else if (!maxBtn.hovered && !closeBtn.hovered)
                                tooltipWindow.visible = false
                        }
                        onClicked: titleBarRoot.settingsClicked()
                    }

                    // 单组件模式下的返回按钮（与设置按钮切换）
                    Button {
                        id: backBtn
                        visible: !titleBarRoot.showTabBar && titleBarRoot.showBackButton
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
                            if (hovered && titleBarRoot.backTooltip.length > 0) {
                                tooltipWindow.tipText = titleBarRoot.backTooltip
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
                                tooltipWindow.tipText = titleBarRoot.isMaximized ? titleBarRoot.restoreTooltip : titleBarRoot.maximizeTooltip
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
                                tooltipWindow.tipText = titleBarRoot.closeTooltip
                                var pt = closeBtn.mapToGlobal(0, closeBtn.height + 4)
                                tooltipWindow.x = pt.x
                                tooltipWindow.y = pt.y
                                tooltipWindow.visible = true
                            } else if (!settingsBtn.hovered && !maxBtn.hovered)
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

        // 第二层：标签栏（背景与第一层标题栏一致；组件标签仍为上圆角样式）
        Rectangle {
            id: secondLayer
            Layout.fillWidth: true
            Layout.preferredHeight: tabBarHeight
            visible: titleBarRoot.showTabBar
            color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.titleBarBackground : "#ffffff"
            clip: true

            readonly property real tabTopRadius: 10
            readonly property real tabMinWidth: 112

            function tabStripColor() {
                if (typeof appController !== "undefined" && appController && appController.themeColors) {
                    var tc = appController.themeColors
                    if (tc.tabInactiveBackground !== undefined && String(tc.tabInactiveBackground).length > 0)
                        return tc.tabInactiveBackground
                    return tc.titleBarBackground
                }
                return "#ffffff"
            }
            function tabActiveColor() {
                if (typeof appController !== "undefined" && appController && appController.themeColors)
                    return appController.themeColors.tabActiveBackground
                return "#ffffff"
            }
            function tabHoverColor() {
                if (typeof appController !== "undefined" && appController && appController.themeColors)
                    return appController.themeColors.tabHoverBackground
                return "#f0f0f0"
            }
            function textPrimaryColor() {
                if (typeof appController !== "undefined" && appController && appController.themeColors)
                    return appController.themeColors.textPrimary
                return "#323232"
            }
            function textSecondaryColor() {
                if (typeof appController !== "undefined" && appController && appController.themeColors)
                    return appController.themeColors.textSecondary
                return "#666666"
            }
            function closeHoverColor() {
                if (typeof appController !== "undefined" && appController && appController.themeColors)
                    return appController.themeColors.tabCloseButtonHover
                return "#e0e0e0"
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 6
                anchors.rightMargin: 6
                anchors.bottomMargin: 0
                spacing: 2

                // 返回主界面：仅图标尺寸 + 少量内边距，悬停圆角底
                Item {
                    id: homeTabItem
                    readonly property int homeIconSize: 16
                    readonly property int homeHPadding: 8
                    Layout.preferredWidth: homeIconSize + homeHPadding * 2
                    Layout.fillHeight: true

                    Rectangle {
                        anchors.fill: parent
                        anchors.topMargin: 4
                        anchors.bottomMargin: 4
                        anchors.leftMargin: 2
                        anchors.rightMargin: 2
                        radius: 6
                        color: backToDesktopBtn.containsMouse ? secondLayer.tabHoverColor() : "transparent"
                    }

                    Image {
                        id: homeIcon
                        anchors.centerIn: parent
                        source: (typeof appController !== "undefined" && appController && appController.theme === 1)
                            ? "qrc:/icons/home_light.svg"
                            : "qrc:/icons/home.svg"
                        sourceSize.width: homeTabItem.homeIconSize
                        sourceSize.height: homeTabItem.homeIconSize
                        width: homeTabItem.homeIconSize
                        height: homeTabItem.homeIconSize
                    }

                    MouseArea {
                        id: backToDesktopBtn
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        z: 1
                        onHoveredChanged: {
                            if (hovered && titleBarRoot.backTooltip.length > 0) {
                                tooltipWindow.tipText = titleBarRoot.backTooltip
                                var pt = backToDesktopBtn.mapToGlobal(0, backToDesktopBtn.height + 4)
                                tooltipWindow.x = pt.x
                                tooltipWindow.y = pt.y
                                tooltipWindow.visible = true
                            } else {
                                tooltipWindow.visible = false
                            }
                        }
                        onClicked: titleBarRoot.switchToDesktop()
                    }
                }

                Repeater {
                    model: typeof appController !== "undefined" && appController ? appController.componentTabs : []

                    Item {
                        id: compTabItem
                        Layout.preferredWidth: Math.max(secondLayer.tabMinWidth, compTabContent.implicitWidth + 28)
                        Layout.fillHeight: true
                        Layout.minimumWidth: secondLayer.tabMinWidth

                        readonly property bool isActiveTab: modelData.isActive && !titleBarRoot.isOnDesktop
                        property color fillColor: {
                            if (isActiveTab)
                                return secondLayer.tabActiveColor()
                            if (tabMouseArea.containsMouse)
                                return secondLayer.tabHoverColor()
                            return secondLayer.tabStripColor()
                        }

                        Shape {
                            anchors.fill: parent
                            antialiasing: true
                            ShapePath {
                                strokeWidth: 0
                                strokeColor: "transparent"
                                fillColor: compTabItem.fillColor
                                startX: 0
                                startY: compTabItem.height
                                PathLine { x: 0; y: secondLayer.tabTopRadius }
                                PathQuad { x: secondLayer.tabTopRadius; y: 0; controlX: 0; controlY: 0 }
                                PathLine { x: compTabItem.width - secondLayer.tabTopRadius; y: 0 }
                                PathQuad { x: compTabItem.width; y: secondLayer.tabTopRadius; controlX: compTabItem.width; controlY: 0 }
                                PathLine { x: compTabItem.width; y: compTabItem.height }
                                PathLine { x: 0; y: compTabItem.height }
                            }
                        }

                        RowLayout {
                            id: compTabContent
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.leftMargin: 12
                            anchors.rightMargin: 6
                            anchors.topMargin: 5
                            anchors.bottomMargin: 1
                            spacing: 8

                            Image {
                                source: modelData.iconPath || "qrc:/icons/app_default.svg"
                                sourceSize.width: 16
                                sourceSize.height: 16
                                Layout.preferredWidth: 16
                                Layout.preferredHeight: 16
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Label {
                                text: modelData.name || modelData.appId
                                font.pixelSize: 12
                                font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                                color: secondLayer.textPrimaryColor()
                                Layout.alignment: Qt.AlignVCenter
                                Layout.maximumWidth: 200
                                elide: Text.ElideRight
                            }

                            Item {
                                Layout.preferredWidth: 12
                                Layout.preferredHeight: 1
                            }

                            Item {
                                Layout.preferredWidth: modelData.isOpened === true ? 22 : 0
                                Layout.preferredHeight: 22
                                Layout.alignment: Qt.AlignVCenter
                                visible: modelData.isOpened === true

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 4
                                    color: closeTabBtn.containsMouse ? secondLayer.closeHoverColor() : "transparent"

                                    Text {
                                        anchors.centerIn: parent
                                        text: "\u2715"
                                        font.pixelSize: 11
                                        font.weight: Font.Medium
                                        color: secondLayer.textSecondaryColor()
                                    }

                                    MouseArea {
                                        id: closeTabBtn
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        z: 2
                                        onClicked: {
                                            tooltipWindow.visible = false
                                            titleBarRoot.tabCloseClicked(modelData.appId)
                                        }
                                        onEntered: tooltipWindow.visible = false
                                    }
                                }
                            }
                        }

                        Rectangle {
                            visible: compTabItem.isActiveTab
                            z: 1
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 2
                            color: "#1A73E8"
                        }

                        MouseArea {
                            id: tabMouseArea
                            anchors.fill: parent
                            anchors.rightMargin: modelData.isOpened === true ? 28 : 0
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            acceptedButtons: Qt.LeftButton
                            z: 2
                            onHoveredChanged: {
                                if (hovered)
                                    tooltipWindow.visible = false
                            }
                            onClicked: titleBarRoot.tabClicked(modelData.appId)
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }

    // 底部分隔线（置于内容之上，避免被 ColumnLayout 铺满盖住）
    Rectangle {
        z: 10
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.titleBarBorder : "#e0e0e0"
    }
}
