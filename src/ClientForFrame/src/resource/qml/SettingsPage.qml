import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent

    readonly property bool isDark: typeof appController !== "undefined" && appController && appController.theme === 1
    readonly property color trackOff: isDark ? "#3c3c3c" : "#e0e0e0"
    readonly property color trackOn: "#007acc"
    readonly property color thumbColor: "#ffffff"

    // 主题相关颜色
    readonly property color comboBoxBg: isDark ? "#2d2d2d" : "#ffffff"
    readonly property color comboBoxBorder: isDark ? "#3c3c3c" : "#e0e0e0"
    readonly property color comboBoxText: isDark ? "#e0e0e0" : "#323232"
    readonly property color comboBoxPopupBg: isDark ? "#2d2d2d" : "#ffffff"
    readonly property color comboBoxHover: isDark ? "#3c3c3c" : "#f0f0f0"

    // 翻译文本
    property string settingsTitle: qsTr("设置")
    property string languageLabelZh: qsTr("语言切换")
    property string languageLabelEn: qsTr("language switch")
    property string languageLabel: (typeof appController !== "undefined" && appController && appController.currentLanguage === 1) ? languageLabelEn : languageLabelZh
    property string chineseText: qsTr("中文")
    property string englishText: qsTr("English")
    property string lightThemeText: qsTr("皮肤模式 [浅色模式]")
    property string darkThemeText: qsTr("皮肤模式 [深色模式]")

    // 监听语言变化并重新计算翻译
    Connections {
        target: typeof appController !== "undefined" ? appController : null
        enabled: typeof appController !== "undefined" && appController !== null
        function onCurrentLanguageChanged() {
            settingsTitle = qsTr("设置")
            languageLabelZh = qsTr("语言切换")
            languageLabelEn = qsTr("language switch")
            languageLabel = (appController.currentLanguage === 1) ? languageLabelEn : languageLabelZh
            chineseText = qsTr("中文")
            englishText = qsTr("English")
            lightThemeText = qsTr("皮肤模式 [浅色模式]")
            darkThemeText = qsTr("皮肤模式 [深色模式]")
        }
    }

    Rectangle {
        anchors.fill: parent
        color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.contentBackground : "#ffffff"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 24
        anchors.bottomMargin: 24
        spacing: 24

        Label {
            text: root.settingsTitle
            font.pixelSize: 17
            font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
            color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 10
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.border : "#e0e0e0"
        }

        // 语言设置
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 40  // 固定行高
            Layout.leftMargin: 10
            spacing: 10
            Label {
                text: root.languageLabel
                font.pixelSize: 13
                font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 120
                Layout.minimumWidth: 120
                Layout.preferredHeight: 40
                verticalAlignment: Text.AlignVCenter
            }
            Item { Layout.preferredWidth: 10 }
            ComboBox {
                id: languageComboBox
                Layout.preferredWidth: 160
                Layout.preferredHeight: 32
                Layout.alignment: Qt.AlignVCenter
                topPadding: 0
                bottomPadding: 0
                model: [root.chineseText, root.englishText]
                currentIndex: typeof appController !== "undefined" && appController ? appController.currentLanguage : 0

                // 主题样式
                background: Rectangle {
                    color: root.comboBoxBg
                    border.color: root.comboBoxBorder
                    border.width: 1
                    radius: 4
                }

                contentItem: Text {
                    text: languageComboBox.displayText
                    font.pixelSize: 14
                    font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                    color: root.comboBoxText
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    leftPadding: 12
                    rightPadding: 36
                }

                // 下拉箭头
                indicator: Canvas {
                    id: canvas
                    x: languageComboBox.width - width - 12
                    y: (languageComboBox.height - height) / 2
                    width: 10
                    height: 6

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);
                        ctx.beginPath();
                        ctx.moveTo(0, 0);
                        ctx.lineTo(width, 0);
                        ctx.lineTo(width / 2, height);
                        ctx.closePath();
                        ctx.fillStyle = root.comboBoxText;
                        ctx.fill();
                    }

                    // 监听主题变化并重绘
                    Connections {
                        target: root
                        onIsDarkChanged: canvas.requestPaint()
                    }
                }

                onActivated: function(index) {
                    if (appController)
                        appController.setLanguage(index)
                }

                // 下拉框样式
                popup: Popup {
                    y: languageComboBox.height - 1
                    width: languageComboBox.width
                    implicitHeight: contentItem.implicitHeight + 8

                    background: Rectangle {
                        color: root.comboBoxPopupBg
                        border.color: root.comboBoxBorder
                        border.width: 1
                        radius: 4
                    }

                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: languageComboBox.popup.visible ? languageComboBox.delegateModel : null
                        currentIndex: languageComboBox.highlightedIndex
                        boundsBehavior: Flickable.StopAtBounds

                        ScrollIndicator.vertical: ScrollIndicator { }
                    }
                }

                delegate: ItemDelegate {
                    width: languageComboBox.width - 2
                    height: 30
                    highlighted: languageComboBox.highlightedIndex === index
                    padding: 0
                    topPadding: 0
                    bottomPadding: 0

                    contentItem: Text {
                        text: modelData
                        font.pixelSize: 13
                        font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                        color: root.comboBoxText
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 12
                    }

                    background: Rectangle {
                        color: parent.highlighted ? root.comboBoxHover : "transparent"
                    }
                }
            }
        }

        // 主题设置
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 40  // 固定行高，与语言设置一致
            Layout.leftMargin: 10
            spacing: 10
            Label {
                text: root.isDark ? root.darkThemeText : root.lightThemeText
                font.pixelSize: 13
                font.family: "Segoe UI, SF Pro Text, Helvetica Neue, Microsoft YaHei UI, sans-serif"
                color: typeof appController !== "undefined" && appController && appController.themeColors ? appController.themeColors.textPrimary : "#323232"
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 120
                Layout.minimumWidth: 120
                Layout.preferredHeight: 40
                verticalAlignment: Text.AlignVCenter
            }
            Item { Layout.preferredWidth: 10 }
            Item {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 22
                Layout.alignment: Qt.AlignVCenter

                property bool checked: root.isDark

                Rectangle {
                    id: track
                    anchors.fill: parent
                    radius: height / 2
                    color: parent.checked ? root.trackOn : root.trackOff
                    border.width: 0
                    Behavior on color { ColorAnimation { duration: 120 } }
                }
                Rectangle {
                    id: thumb
                    width: 18
                    height: 18
                    radius: width / 2
                    y: (parent.height - height) / 2
                    x: parent.checked ? (parent.width - width - 2) : 2
                    color: root.thumbColor
                    Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (appController)
                            appController.setTheme(parent.checked ? 0 : 1)
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
