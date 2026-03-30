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
        anchors.fill: parent
        // 与设置里「语言」一致（TranslationManager：0=中文，1=英文）；无框架上下文时 -1 回退系统区域
        frameworkLanguage: (typeof appController !== "undefined" && appController) ? appController.currentLanguage : -1
    }

    Component.onCompleted: {
        if (typeof appController !== "undefined" && appController) {
            appController.registerComponentHost(playerHostItem)
            Qt.callLater(function () {
                if (appController && playerHostItem.running)
                    appController.setComponentRuntimeActive(true)
            })
        }
    }

    Component.onDestruction: {
        if (typeof appController !== "undefined" && appController)
            appController.unregisterComponentHost()
    }

    Connections {
        target: playerHostItem
        function onRunningChanged() {
            if (typeof appController !== "undefined" && appController)
                appController.setComponentRuntimeActive(playerHostItem.running)
        }
    }

    onVisibleChanged: {
        if (!visible) {
            playerHostItem.stop()
        }
    }
}
