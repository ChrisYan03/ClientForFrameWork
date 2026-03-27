// 主窗口：框架主体 = 标题栏 + 内容区（桌面 / 独立组件）
// 内容区为 StackView：默认显示桌面（App 图标），点击图标加载对应组件（如图像匹配）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import App 1.0

Window {
    id: root
    visible: false
    title: root.appTitle
    width: 1400
    height: 900
    minimumWidth: 1000
    minimumHeight: 700
    flags: Qt.Window | Qt.FramelessWindowHint
    color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.windowBackground : "#f3f3f3"

    property string mainStatusText: ""
    property bool isMaximized: visibility === Window.Maximized
    property bool toastVisible: false
    property string toastMessage: root.pauseMessage
    property bool settingsOpen: false
    readonly property int titleBarHeight: (typeof appController !== "undefined" && appController && appController.componentCount > 1) ? 78 : 38
    readonly property int cornerRadius: 8
    readonly property int contentMargin: 0

    // 翻译文本
    property string appTitle: qsTr("小闫客户端")
    property string pauseMessage: qsTr("请暂停后再回到主界面")
    property string stopConfigMessage: qsTr("停止运行后可配置")

    // 监听语言变化
    Connections {
        target: typeof appController !== "undefined" ? appController : null
        function onCurrentLanguageChanged() {
            appTitle = qsTr("小闫客户端")
            pauseMessage = qsTr("请暂停后再回到主界面")
            stopConfigMessage = qsTr("停止运行后可配置")
        }
    }

    Rectangle {
        id: roundBack
        anchors.fill: parent
        radius: root.isMaximized ? 0 : root.cornerRadius
        color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.contentBackground : "#ffffff"
        border.width: root.isMaximized ? 0 : 1
        border.color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.border : "#e0e0e0"
        clip: true

        TitleBar {
            id: titleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: root.titleBarHeight
            statusText: root.mainStatusText
            isMaximized: root.isMaximized
            showBackButton: contentStack.depth > 1
            showSettingsButton: contentStack.depth <= 1
            isOnDesktop: contentStack.depth <= 1
            onRequestMove: (dx, dy) => {
                if (!root.isMaximized)
                    root.x += dx; root.y += dy
            }
            onRequestMaximize: {
                if (root.visibility === Window.Maximized)
                    root.showNormal()
                else
                    root.showMaximized()
            }
            onBackToDesktopClicked: {
                if (contentStack.depth > 1) {
                    if (appController && appController.hasRunnableComponent && appController.isRunning) {
                        root.showBubbleMessage(root.pauseMessage)
                    } else if (appController) {
                        root.closeCurrentTab()
                    }
                }
            }
            onSwitchToDesktop: {
                // 切换到主界面（不关闭组件，只是切换视图）
                root.popToDesktop()
            }
            onSettingsClicked: {
                if (appController && appController.hasRunnableComponent && appController.isRunning) {
                    root.showBubbleMessage(root.stopConfigMessage)
                    return
                }
                root.settingsOpen = !root.settingsOpen
            }
            onTabClicked: (appId) => {
                root.switchToComponentTab(appId)
            }
            onTabCloseClicked: (appId) => {
                root.closeComponentTab(appId)
            }
        }

        Timer {
            id: toastTimer
            interval: 2000
            repeat: false
            onTriggered: root.toastVisible = false
        }

        Item {
            id: contentArea
            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: root.contentMargin
            anchors.rightMargin: root.contentMargin
            anchors.bottomMargin: root.contentMargin

            StackView {
                id: contentStack
                anchors.top: contentArea.top
                anchors.bottom: contentArea.bottom
                anchors.left: contentArea.left
                anchors.right: root.settingsOpen ? settingsPanel.left : contentArea.right

                Component.onCompleted: {
                    push(appDesktopComp)
                }
            }

            Item {
                id: settingsPanel
                anchors.top: contentArea.top
                anchors.right: contentArea.right
                anchors.bottom: contentArea.bottom
                width: root.settingsOpen ? (contentArea.width / 4) : 0
                visible: root.settingsOpen || width > 0
                clip: true

                Behavior on width {
                    NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
                }

                Rectangle {
                    anchors.fill: parent
                    color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.contentBackground : "#ffffff"
                }
                Rectangle {
                    width: 1
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.border : "#e0e0e0"
                }

                Loader {
                    id: settingsLoader
                    anchors.fill: parent
                    anchors.leftMargin: 1
                    // macOS 上若在 width 为 0 时激活，加载的页面会保持 0 尺寸显示空白；等面板有足够宽度后再加载
                    active: root.settingsOpen && settingsPanel.width > 80
                    sourceComponent: settingsPageComp
                }
            }
        }

        Component {
            id: appDesktopComp
            AppDesktop {
                onOpenApp: (appId) => root.openAppFromDesktop(appId)
            }
        }
        Component {
            id: settingsPageComp
            SettingsPage { }
        }
    }

    function openAppFromDesktop(appId) {
        root.settingsOpen = false
        if (!appController)
            return

        // 判断是否为多组件模式
        if (appController.componentCount > 1) {
            // 多组件模式：使用标签页管理
            var idx = appController.openComponentTab(appId)
            if (idx >= 0) {
                root.loadComponentPage(appId)
            }
        } else {
            // 单组件模式：保持原有行为
            var pageUrl = appController.getComponentPageUrl(appId)
            if (pageUrl && pageUrl.toString()) {
                contentStack.push(pageUrl)
                var componentTitle = appController.getComponentName(appId)
                appController.setPageTitle(componentTitle && componentTitle !== "" ? componentTitle : root.appTitle)
            }
        }
    }

    function loadComponentPage(appId) {
        var pageUrl = appController.getComponentPageUrl(appId)
        if (pageUrl && pageUrl.toString()) {
            // 先返回桌面
            while (contentStack.depth > 1)
                contentStack.pop()
            // 再加载组件页面
            contentStack.push(pageUrl)
        }
    }

    function switchToComponentTab(appId) {
        if (!appController)
            return
        appController.switchToTab(appId)
        root.loadComponentPage(appId)
    }

    function closeComponentTab(appId) {
        if (!appController)
            return
        // 如果关闭的是当前标签且组件正在运行，先提示停止
        if (appId === appController.currentTabAppId() && appController.hasRunnableComponent && appController.isRunning) {
            root.showBubbleMessage(root.pauseMessage)
            return
        }
        // 如果关闭的是当前标签，先注销组件
        if (appId === appController.currentTabAppId()) {
            appController.unregisterComponentHost()
        }
        appController.closeComponentTab(appId)
        // 返回桌面
        while (contentStack.depth > 1)
            contentStack.pop()
        // 如果还有打开的标签，切换到第一个；否则就留在桌面
        var currentAppId = appController.currentTabAppId()
        if (currentAppId && currentAppId !== "") {
            root.loadComponentPage(currentAppId)
        }
        // 所有标签关闭后，已在桌面，不需要额外操作
    }

    function closeCurrentTab() {
        if (!appController)
            return
        var currentAppId = appController.currentTabAppId()
        if (currentAppId && currentAppId !== "") {
            appController.unregisterComponentHost()
            appController.closeComponentTab(currentAppId)
        }
        // 返回桌面
        while (contentStack.depth > 1)
            contentStack.pop()
        // 如果还有打开的标签，切换到第一个
        var nextAppId = appController.currentTabAppId()
        if (nextAppId && nextAppId !== "") {
            root.loadComponentPage(nextAppId)
        }
        // 所有标签关闭后，已在桌面
    }

    function popToDesktop() {
        while (contentStack.depth > 1)
            contentStack.pop()
    }
    function showBubbleMessage(msg) {
        root.toastMessage = msg
        root.toastVisible = true
        toastTimer.restart()
    }

    Component.onCompleted: {
        if (appController)
            root.mainStatusText = appController.statusText
        if (root.visibility !== Window.Maximized) {
            var screen = Qt.application.screens.length > 0 ? Qt.application.screens[0] : null
            if (screen) {
                root.x = (screen.width - root.width) / 2
                root.y = (screen.height - root.height) / 2
            }
        }
    }
    Connections {
        target: appController
        function onStatusTextChanged() {
            if (appController)
                root.mainStatusText = appController.statusText
        }
        function onShowBubbleMessageRequested(msg) {
            root.showBubbleMessage(msg)
        }
        function onBackToDesktopRequested() {
            if (appController)
                appController.unregisterComponentHost()
            root.popToDesktop()
        }
        function onCurrentTabChanged(appId) {
            // 当标签切换时，加载对应的组件页面
            if (appId && appId !== "") {
                root.loadComponentPage(appId)
            }
        }
    }

    // 气泡提示：主框架标题栏下 40px 显示（无尖角）
    Window {
        id: bubbleWindow
        flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
        color: "transparent"
        width: 260
        height: 48
        x: root.x + (root.width - width) / 2
        y: root.y + root.titleBarHeight + 40
        visible: root.toastVisible

        Rectangle {
            anchors.fill: parent
            radius: 6
            color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.tooltipBackground : "#ffffff"
            border.width: 1
            border.color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.tooltipBorder : "#dadce0"

            Label {
                anchors.centerIn: parent
                text: root.toastMessage
                font.pixelSize: 13
                font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.toastVisible = false
            }
        }
    }
}
