// 图像匹配组件页：嵌入 PicMatchWidget，加载时向框架注册以便标题栏启动/停止控制本组件
import QtQuick
import QtQuick.Layouts
import App 1.0

Item {
    id: root
    objectName: "picMatchPage"

    property bool isActive: true

    Component.onCompleted: {
        if (typeof appController !== "undefined" && appController)
            appController.registerComponentHost(playerHost)
    }
    Component.onDestruction: {
        if (typeof appController !== "undefined" && appController)
            appController.unregisterComponentHost()
    }

    PlayerHostItem {
        id: playerHost
        objectName: "playerHost"
        anchors.fill: parent
    }
}
