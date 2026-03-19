import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.1

ApplicationWindow {
    id: root
    width: 1500
    height: 920
    visible: true
    title: "tools_inside"
    color: root.appBg

    property real timelineScrollX: 0
    property int laneLabelWidth: 220
    property real manualPixelsPerTick: 24
    property bool fitTimeline: false
    property int tickCount: Math.max(2, Math.ceil(toolsInsideBrowser.timelineDurationMs / Math.max(1, toolsInsideBrowser.timelineTickMs)) + 1)
    property string selectedTimelineKey: ""
    property string selectedToolCallId: ""
    property string selectedSupportLinkId: ""
    property string selectedArtifactId: ""
    property string themeMode: "win11"
    property string contrastMode: "normal"
    property string languageMode: "zh"
    property color appBg: themeMode === "slate" ? "#dde3e8" : (themeMode === "win11" ? "#f3f5f7" : "#eee5d8")
    property color contentBg: themeMode === "slate" ? "#e7edf2" : (themeMode === "win11" ? "#f7f9fb" : "#f4efe6")
    property color panelBg: themeMode === "slate" ? "#f6f8fb" : (themeMode === "win11" ? "#ffffff" : "#fffaf2")
    property color panelAltBg: themeMode === "slate" ? "#edf2f7" : (themeMode === "win11" ? "#f5f7fa" : "#f7efe3")
    property color panelBorder: contrastMode === "high" ? (themeMode === "slate" ? "#6b7d8d" : (themeMode === "win11" ? "#7f8b99" : "#9f8563")) : (themeMode === "slate" ? "#c6d1db" : (themeMode === "win11" ? "#d7dde5" : "#d8ccb8"))
    property color headerBarBg: themeMode === "slate" ? "#eaf0f5" : (themeMode === "win11" ? "#ffffff" : "#f7f0e5")
    property color headerText: contrastMode === "high" ? "#11161a" : "#18222a"
    property color bodyText: contrastMode === "high" ? "#18222a" : "#31414a"
    property color mutedText: contrastMode === "high" ? "#3f505a" : (themeMode === "win11" ? "#5f6b77" : "#6a787f")
    property color accentBg: themeMode === "slate" ? "#d8e2ec" : (themeMode === "win11" ? "#e9f1fb" : "#efe1cf")
    property color accentBorder: contrastMode === "high" ? (themeMode === "slate" ? "#567089" : (themeMode === "win11" ? "#6d8eaf" : "#9c7440")) : (themeMode === "slate" ? "#c1cfdd" : (themeMode === "win11" ? "#c7d8ea" : "#d5c2a8"))
    property real uiScale: 1.0
    property int titleFontPx: Math.round(22 * uiScale)
    property int sectionFontPx: Math.round(18 * uiScale)
    property int bodyFontPx: Math.round(12 * uiScale)
    property int smallFontPx: Math.round(11 * uiScale)
    property int metricFontPx: Math.round(26 * uiScale)

    function effectivePixelsPerTick(viewWidth) {
        if (fitTimeline) {
            return Math.max(18, (Math.max(200, viewWidth - laneLabelWidth - 8)) / Math.max(1, tickCount - 1))
        }
        return manualPixelsPerTick
    }

    function zoomIn() {
        fitTimeline = false
        manualPixelsPerTick = Math.min(144, manualPixelsPerTick * 1.25)
    }

    function zoomOut() {
        fitTimeline = false
        manualPixelsPerTick = Math.max(6, manualPixelsPerTick / 1.25)
    }

    function resetZoom() {
        fitTimeline = false
        manualPixelsPerTick = 24
    }

    function timelineEntryKey(entry) {
        return (entry.laneId || "")
                + "|" + String(entry.startMs || 0)
                + "|" + String(entry.endMs || 0)
                + "|" + (entry.label || "")
                + "|" + (entry.entryType || "")
    }

    function uiText(en, zh) {
        return languageMode === "zh" ? zh : en
    }

    Settings {
        category: "display"
        property alias uiScale: root.uiScale
        property alias manualPixelsPerTick: root.manualPixelsPerTick
        property alias fitTimeline: root.fitTimeline
        property alias themeMode: root.themeMode
        property alias contrastMode: root.contrastMode
        property alias languageMode: root.languageMode
    }

    Popup {
        id: settingsPopup
        x: Math.max(16, root.width - width - 24)
        y: 60
        width: 280
        modal: false
        focus: true
        padding: 0
        background: Rectangle {
            radius: 14
            color: root.panelBg
            border.color: root.panelBorder
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 12

            Label {
                text: root.uiText("Display Settings", "显示设置")
                font.pixelSize: root.sectionFontPx
                font.bold: true
                color: root.headerText
            }

            Label {
                text: root.uiText("Font Scale", "字体缩放")
                color: root.bodyText
                font.pixelSize: root.bodyFontPx
            }

            ComboBox {
                Layout.fillWidth: true
                model: [
                    { text: "90%", value: 0.9 },
                    { text: "100%", value: 1.0 },
                    { text: "110%", value: 1.1 },
                    { text: "125%", value: 1.25 },
                    { text: "140%", value: 1.4 }
                ]
                textRole: "text"
                Component.onCompleted: {
                    for (var i = 0; i < model.length; ++i) {
                        if (Math.abs(model[i].value - root.uiScale) < 0.001) {
                            currentIndex = i
                            break
                        }
                    }
                }
                onActivated: root.uiScale = model[index].value
            }

            Label {
                text: root.uiText("Theme", "主题")
                color: root.bodyText
                font.pixelSize: root.bodyFontPx
            }

            ComboBox {
                Layout.fillWidth: true
                model: [
                    { text: root.uiText("Windows 11 Light", "Windows 11 浅色"), value: "win11" },
                    { text: root.uiText("Sand", "暖沙"), value: "sand" },
                    { text: root.uiText("Slate", "石板"), value: "slate" }
                ]
                textRole: "text"
                Component.onCompleted: {
                    for (var i = 0; i < model.length; ++i) {
                        if (model[i].value === root.themeMode) {
                            currentIndex = i
                            break
                        }
                    }
                }
                onActivated: root.themeMode = model[index].value
            }

            Label {
                text: root.uiText("Contrast", "对比度")
                color: root.bodyText
                font.pixelSize: root.bodyFontPx
            }

            ComboBox {
                Layout.fillWidth: true
                model: [
                    { text: root.uiText("Normal", "标准"), value: "normal" },
                    { text: root.uiText("High", "高对比"), value: "high" }
                ]
                textRole: "text"
                Component.onCompleted: {
                    for (var i = 0; i < model.length; ++i) {
                        if (model[i].value === root.contrastMode) {
                            currentIndex = i
                            break
                        }
                    }
                }
                onActivated: root.contrastMode = model[index].value
            }

            Label {
                text: root.uiText("Language", "界面语言")
                color: root.bodyText
                font.pixelSize: root.bodyFontPx
            }

            ComboBox {
                Layout.fillWidth: true
                model: [
                    { text: "English", value: "en" },
                    { text: "中文", value: "zh" }
                ]
                textRole: "text"
                Component.onCompleted: {
                    for (var i = 0; i < model.length; ++i) {
                        if (model[i].value === root.languageMode) {
                            currentIndex = i
                            break
                        }
                    }
                }
                onActivated: root.languageMode = model[index].value
            }

            Label {
                text: root.uiText(
                          "Timeline zoom stays on the main toolbar. This panel now controls font scale, theme, contrast, and language, and can be extended later.",
                          "时间线缩放仍保留在主工具栏。这个面板当前可控制字体缩放、主题、对比度和界面语言，后续可继续扩展。")
                wrapMode: Text.Wrap
                color: root.mutedText
                font.pixelSize: root.smallFontPx
                Layout.fillWidth: true
            }
        }
    }

    Popup {
        id: workspacePopup
        x: workspaceButton.mapToItem(root.contentItem, 0, 0).x
        y: workspaceButton.mapToItem(root.contentItem, 0, workspaceButton.height + 8).y
        width: 420
        height: 420
        modal: false
        focus: true
        padding: 0
        background: Rectangle {
            radius: 14
            color: root.panelBg
            border.color: root.panelBorder
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: root.uiText("Workspaces", "工作区")
                    font.pixelSize: root.sectionFontPx
                    font.bold: true
                    color: root.headerText
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: toolsInsideBrowser.isFavoriteWorkspace(toolsInsideBrowser.workspaceRoot)
                          ? root.uiText("Unpin Current", "取消固定当前目录")
                          : root.uiText("Pin Current", "固定当前目录")
                    onClicked: toolsInsideBrowser.toggleWorkspaceFavorite(toolsInsideBrowser.workspaceRoot)
                }
                Button {
                    text: root.uiText("Clear Recent", "清空最近目录")
                    onClicked: toolsInsideBrowser.clearWorkspaceHistory()
                }
            }

            Label {
                text: root.uiText("Favorites", "常用目录")
                color: root.bodyText
                font.pixelSize: root.bodyFontPx
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.preferredHeight: 140
                clip: true
                background: Rectangle {
                    radius: 10
                    color: root.panelAltBg
                    border.color: root.panelBorder
                }

                Column {
                    width: parent.width
                    spacing: 6

                    Repeater {
                        model: toolsInsideBrowser.workspaceFavorites
                        delegate: Rectangle {
                            width: parent.width
                            height: 38
                            radius: 8
                            color: modelData === toolsInsideBrowser.workspaceRoot ? "#e3efe9" : "#fcf8f1"
                            border.color: root.panelBorder

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                spacing: 6

                                Button {
                                    text: root.uiText("Open", "打开")
                                    onClicked: toolsInsideBrowser.selectWorkspaceHistory(modelData)
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: modelData
                                    elide: Label.ElideMiddle
                                    color: root.bodyText
                                    font.pixelSize: root.smallFontPx
                                }

                                Button {
                                    text: root.uiText("Remove", "移除")
                                    onClicked: toolsInsideBrowser.removeWorkspaceFavorite(modelData)
                                }
                            }
                        }
                    }

                    Label {
                        visible: toolsInsideBrowser.workspaceFavorites.length === 0
                        text: root.uiText("No pinned workspaces yet.", "还没有固定的工作区。")
                        color: root.mutedText
                        font.pixelSize: root.smallFontPx
                    }
                }
            }

            Label {
                text: root.uiText("Recent", "最近目录")
                color: root.bodyText
                font.pixelSize: root.bodyFontPx
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                background: Rectangle {
                    radius: 10
                    color: root.panelAltBg
                    border.color: root.panelBorder
                }

                Column {
                    width: parent.width
                    spacing: 6

                    Repeater {
                        model: toolsInsideBrowser.workspaceHistory
                        delegate: Rectangle {
                            width: parent.width
                            height: 36
                            radius: 8
                            color: modelData === toolsInsideBrowser.workspaceRoot ? "#f1debf" : "#fcf8f1"
                            border.color: root.panelBorder

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                spacing: 6

                                Button {
                                    text: root.uiText("Open", "打开")
                                    onClicked: toolsInsideBrowser.selectWorkspaceHistory(modelData)
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: modelData
                                    elide: Label.ElideMiddle
                                    color: root.bodyText
                                    font.pixelSize: root.smallFontPx
                                }

                                Button {
                                    text: toolsInsideBrowser.isFavoriteWorkspace(modelData)
                                          ? root.uiText("Unpin", "取消固定")
                                          : root.uiText("Pin", "固定")
                                    onClicked: toolsInsideBrowser.toggleWorkspaceFavorite(modelData)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    header: ToolBar {
        contentHeight: 48
        background: Rectangle {
            color: root.headerBarBg
            border.color: root.panelBorder
        }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            Button {
                id: workspaceButton
                text: root.uiText("Workspaces", "工作区")
                onClicked: workspacePopup.open()
            }

            Label {
                text: "tools_inside"
                font.pixelSize: root.titleFontPx
                font.bold: true
                color: root.headerText
            }

            ComboBox {
                id: historyCombo
                Layout.fillWidth: true
                Layout.preferredWidth: 420
                model: toolsInsideBrowser.workspaceHistory
                enabled: model.length > 0
                currentIndex: Math.max(0, toolsInsideBrowser.workspaceHistory.indexOf(toolsInsideBrowser.workspaceRoot))
                onActivated: toolsInsideBrowser.selectWorkspaceHistory(currentText)
            }

            Button {
                text: "..."
                onClicked: toolsInsideBrowser.chooseWorkspaceRoot()
            }
            Button { text: root.uiText("Reload", "刷新"); onClicked: toolsInsideBrowser.reload() }
            Button { text: "-"; onClicked: root.zoomOut() }
            Button {
                text: root.fitTimeline ? "Fit" : Math.round(root.manualPixelsPerTick) + "px/6ms"
                onClicked: root.resetZoom()
            }
            Button { text: "+"; onClicked: root.zoomIn() }
            Button {
                text: root.uiText("Fit", "适配")
                onClicked: root.fitTimeline = true
            }
            Button {
                text: root.uiText("Settings", "设置")
                onClicked: settingsPopup.open()
            }
            Button {
                text: root.uiText("Archive Trace", "归档 Trace")
                enabled: toolsInsideBrowser.selectedTraceId.length > 0
                onClicked: toolsInsideBrowser.archiveSelectedTrace()
            }
            Button {
                text: root.uiText("Purge Trace", "清除 Trace")
                enabled: toolsInsideBrowser.selectedTraceId.length > 0
                onClicked: toolsInsideBrowser.purgeSelectedTrace()
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.contentBg

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            Rectangle {
                Layout.preferredWidth: 360
                Layout.fillHeight: true
                radius: 18
                color: root.panelBg
                border.color: root.panelBorder

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 10

                    Label { text: root.uiText("Client / Session / Trace", "客户端 / 会话 / Trace"); font.pixelSize: root.sectionFontPx; font.bold: true; color: root.headerText }

                    Label { text: root.uiText("Clients", "客户端"); font.bold: true; color: root.bodyText; font.pixelSize: root.bodyFontPx }
                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 160
                        clip: true
                        model: toolsInsideBrowser.clients
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 52
                            radius: 10
                            color: modelData.clientId === toolsInsideBrowser.selectedClientId ? "#dde9dc" : "#fcf8f1"
                            border.color: modelData.clientId === toolsInsideBrowser.selectedClientId ? "#95b08f" : "#ded4c6"
                            MouseArea { anchors.fill: parent; onClicked: toolsInsideBrowser.selectClient(modelData.clientId) }
                            Column {
                                anchors.fill: parent
                                anchors.margins: 8
                                Text { text: modelData.clientId; font.bold: true; color: root.headerText; elide: Text.ElideRight; width: parent.width }
                                Text { text: modelData.traceCount + " traces / " + modelData.sessionCount + " sessions"; color: root.mutedText }
                            }
                        }
                    }

                    Label { text: root.uiText("Sessions", "会话"); font.bold: true; color: root.bodyText; font.pixelSize: root.bodyFontPx }
                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 180
                        clip: true
                        model: toolsInsideBrowser.sessions
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 48
                            radius: 10
                            color: modelData.sessionId === toolsInsideBrowser.selectedSessionId ? "#efe0c9" : "#fcf8f1"
                            border.color: modelData.sessionId === toolsInsideBrowser.selectedSessionId ? "#c7a97e" : "#ded4c6"
                            MouseArea { anchors.fill: parent; onClicked: toolsInsideBrowser.selectSession(modelData.sessionId) }
                            Column {
                                anchors.fill: parent
                                anchors.margins: 8
                                Text { text: modelData.sessionId; font.bold: true; color: root.headerText; elide: Text.ElideRight; width: parent.width }
                                Text { text: modelData.traceCount + " traces"; color: root.mutedText }
                            }
                        }
                    }

                    Label { text: root.uiText("Traces", "Trace 列表"); font.bold: true; color: root.bodyText; font.pixelSize: root.bodyFontPx }
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: toolsInsideBrowser.traces
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 70
                            radius: 12
                            color: modelData.traceId === toolsInsideBrowser.selectedTraceId ? "#f1debf" : "#fcf8f1"
                            border.color: modelData.traceId === toolsInsideBrowser.selectedTraceId ? "#b77b36" : "#ded4c6"
                            MouseArea { anchors.fill: parent; onClicked: toolsInsideBrowser.selectTrace(modelData.traceId) }
                            Column {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 2
                                Text { text: modelData.traceId; font.bold: true; color: root.headerText; elide: Text.ElideMiddle; width: parent.width }
                                Text { text: modelData.status + " | " + modelData.provider + " / " + modelData.model; color: root.mutedText; elide: Text.ElideRight; width: parent.width }
                                Text { text: modelData.turnInputPreview; color: root.bodyText; elide: Text.ElideRight; width: parent.width }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 20
                color: root.panelBg
                border.color: root.panelBorder

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Label {
                        text: toolsInsideBrowser.selectedTraceId.length > 0 ? root.uiText("Trace Detail", "Trace 详情") : root.uiText("Select a trace", "请选择一个 Trace")
                        font.pixelSize: Math.round(20 * root.uiScale)
                        font.bold: true
                        color: root.headerText
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 104
                            radius: 14
                            color: root.panelAltBg
                            border.color: root.panelBorder
                            Column {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 4
                                Text {
                                    text: root.uiText("Status: ", "状态：") + (toolsInsideBrowser.traceSummary.status || "-")
                                          + " | " + root.uiText("Provider: ", "提供方：") + (toolsInsideBrowser.traceSummary.provider || "-")
                                          + " | " + root.uiText("Model: ", "模型：") + (toolsInsideBrowser.traceSummary.model || "-")
                                    color: root.bodyText
                                }
                                Text { text: root.uiText("Trace: ", "Trace：") + (toolsInsideBrowser.traceSummary.traceId || "-"); color: root.bodyText; elide: Text.ElideMiddle; width: parent.width }
                                Text { text: "T0: " + (toolsInsideBrowser.traceSummary.t0Utc || "-") + " | " + root.uiText("Duration: ", "时长：") + (toolsInsideBrowser.traceSummary.durationMs || 0) + " ms | " + root.uiText("Tick: ", "刻度：") + toolsInsideBrowser.timelineTickMs + " ms"; color: root.bodyText; elide: Text.ElideRight; width: parent.width }
                                Text { text: root.uiText("Input: ", "输入：") + (toolsInsideBrowser.traceSummary.turnInputPreview || "-"); color: root.mutedText; elide: Text.ElideRight; width: parent.width }
                            }
                        }

                        Repeater {
                            model: [
                                { "title": "Lanes", "value": toolsInsideBrowser.traceSummary.laneCount || 0, "tone": "#e8dfd0" },
                                { "title": "Tool Calls", "value": toolsInsideBrowser.toolCalls.length, "tone": "#dfece6" },
                                { "title": "Support", "value": toolsInsideBrowser.supportLinks.length, "tone": "#e5ebf4" },
                                { "title": "Artifacts", "value": toolsInsideBrowser.artifacts.length, "tone": "#f0e4d6" }
                            ]
                            delegate: Rectangle {
                                Layout.preferredWidth: 124
                                Layout.preferredHeight: 104
                                radius: 14
                                color: modelData.tone
                                border.color: "#d2c4b0"
                                Column {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 6
                                    Text { text: modelData.title; color: root.mutedText; font.pixelSize: root.bodyFontPx }
                                    Text { text: String(modelData.value); color: root.headerText; font.pixelSize: root.metricFontPx; font.bold: true }
                                }
                            }
                        }
                    }

                    SplitView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        orientation: Qt.Vertical
                        handle: Item {
                            implicitWidth: parent && parent.orientation === Qt.Horizontal ? 12 : parent ? parent.width : 12
                            implicitHeight: parent && parent.orientation === Qt.Vertical ? 12 : parent ? parent.height : 12
                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width > parent.height ? 56 : 4
                                height: parent.width > parent.height ? 4 : 56
                                radius: 2
                                color: "#a9967a"
                            }
                        }

                        Rectangle {
                            SplitView.fillWidth: true
                            SplitView.preferredHeight: 540
                            SplitView.minimumHeight: 360
                            radius: 16
                            color: root.panelAltBg
                            border.color: root.panelBorder
                            clip: true

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 42
                                    radius: 10
                                    color: root.accentBg
                                    border.color: root.accentBorder

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12
                                        spacing: 8

                                        Label {
                                            text: root.uiText("Swimlane Timeline (T0 + ms, 6ms ticks)", "泳道时间线（T0 + 毫秒，6ms 基础刻度）")
                                            font.bold: true
                                            font.pixelSize: root.bodyFontPx
                                            color: root.headerText
                                        }

                                        Item { Layout.fillWidth: true }

                                        RowLayout {
                                            spacing: 8
                                            Rectangle { width: 14; height: 14; radius: 7; color: "#b67632"; border.color: "#845626" }
                                            Text { text: root.uiText("Request", "模型请求"); color: root.mutedText; font.pixelSize: root.smallFontPx }
                                            Rectangle { width: 14; height: 14; radius: 7; color: "#557ea0"; border.color: "#40627d" }
                                            Text { text: root.uiText("Tool Batch", "工具批次"); color: root.mutedText; font.pixelSize: root.smallFontPx }
                                            Rectangle { width: 14; height: 14; radius: 7; color: "#2f7d73"; border.color: "#235d56" }
                                            Text { text: root.uiText("Tool Call", "工具调用"); color: root.mutedText; font.pixelSize: root.smallFontPx }
                                            Rectangle { width: 12; height: 12; rotation: 45; color: "#fff7ea"; border.color: "#6a5c4d" }
                                            Text { text: root.uiText("Event", "事件"); color: root.mutedText; font.pixelSize: root.smallFontPx }
                                        }
                                    }
                                }

                                Item {
                                    id: chartViewport
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    clip: true
                                    property real leadingGap: 28
                                    property real tickPixels: root.effectivePixelsPerTick(width)
                                    property real pixelsPerMs: tickPixels / Math.max(1, toolsInsideBrowser.timelineTickMs)
                                    property int labelStride: Math.max(1, Math.ceil((String(Math.max(0, (root.tickCount - 1) * toolsInsideBrowser.timelineTickMs)).length * root.smallFontPx * 0.68 + 10) / Math.max(18, tickPixels)))
                                    property real chartWidth: Math.max(width - root.laneLabelWidth - 8,
                                                                       leadingGap + root.tickCount * tickPixels)

                                    Column {
                                        anchors.fill: parent
                                        spacing: 8

                                        Item {
                                            width: parent.width
                                            height: 44

                                            Row {
                                                anchors.fill: parent
                                                spacing: 0

                                                Rectangle {
                                                    width: root.laneLabelWidth
                                                    height: parent.height
                                                    color: "#efe5d7"
                                                    border.color: "#d7c8b2"
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: root.uiText("Lane / ms", "泳道 / 毫秒")
                                                        color: root.bodyText
                                                        font.bold: true
                                                        font.pixelSize: root.bodyFontPx
                                                    }
                                                }

                                                Flickable {
                                                    id: rulerFlick
                                                    width: Math.max(0, parent.width - root.laneLabelWidth)
                                                    height: parent.height
                                                    contentWidth: chartViewport.chartWidth
                                                    contentHeight: height
                                                    clip: true
                                                    flickableDirection: Flickable.HorizontalFlick
                                                    boundsBehavior: Flickable.StopAtBounds
                                                    onContentXChanged: if (movingHorizontally) root.timelineScrollX = contentX

                                                    Binding {
                                                        target: rulerFlick
                                                        property: "contentX"
                                                        value: root.timelineScrollX
                                                        when: !rulerFlick.movingHorizontally
                                                    }

                                                    Rectangle {
                                                        width: chartViewport.chartWidth
                                                        height: rulerFlick.height
                                                        color: "#fbf7f0"

                                                        Repeater {
                                                            model: root.tickCount
                                                            delegate: Item {
                                                                x: chartViewport.leadingGap + index * chartViewport.tickPixels
                                                                width: chartViewport.tickPixels
                                                                height: parent.height

                                                                Rectangle {
                                                                    anchors.left: parent.left
                                                                    anchors.top: parent.top
                                                                    anchors.bottom: parent.bottom
                                                                    width: index % 5 === 0 ? 2 : 1
                                                                    color: index % 5 === 0 ? "#8c6f45" : "#d6c7b1"
                                                                }

                                                                Text {
                                                                    anchors.centerIn: parent
                                                                    width: parent.width - 4
                                                                    horizontalAlignment: Text.AlignHCenter
                                                                    text: String(index * toolsInsideBrowser.timelineTickMs)
                                                                    font.pixelSize: root.smallFontPx
                                                                    color: index % 5 === 0 ? "#664c2d" : root.mutedText
                                                                    elide: Text.ElideRight
                                                                    clip: true
                                                                    visible: index % chartViewport.labelStride === 0
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        ListView {
                                            id: laneList
                                            width: parent.width
                                            height: Math.max(0, parent.height - 52)
                                            clip: true
                                            spacing: 6
                                            model: toolsInsideBrowser.timelineLanes
                                            boundsBehavior: Flickable.StopAtBounds

                                            delegate: RowLayout {
                                                width: laneList.width
                                                height: Math.max(64, 20 + Math.max(1, modelData.rowCount || 1) * 24)
                                                spacing: 0

                                                Rectangle {
                                                    Layout.preferredWidth: root.laneLabelWidth
                                                    Layout.fillHeight: true
                                                    radius: 10
                                                    color: modelData.laneKind === "tool" ? "#e3efe9" : (modelData.laneKind === "batch" ? "#e5edf5" : (modelData.laneKind === "request" ? "#efe5d6" : "#ebe4db"))
                                                    border.color: "#d4c7b6"

                                                    Column {
                                                        anchors.fill: parent
                                                        anchors.margins: 8
                                                        spacing: 2
                                                        Text { text: modelData.title; font.bold: true; color: root.bodyText; elide: Text.ElideRight; width: parent.width }
                                                        Text { text: modelData.subtitle || ""; color: root.mutedText; elide: Text.ElideMiddle; width: parent.width }
                                                    }
                                                }

                                                Flickable {
                                                    id: laneFlick
                                                    Layout.fillWidth: true
                                                    Layout.fillHeight: true
                                                    contentWidth: chartViewport.chartWidth
                                                    contentHeight: height
                                                    clip: true
                                                    flickableDirection: Flickable.HorizontalFlick
                                                    boundsBehavior: Flickable.StopAtBounds
                                                    onContentXChanged: if (movingHorizontally) root.timelineScrollX = contentX

                                                    Binding {
                                                        target: laneFlick
                                                        property: "contentX"
                                                        value: root.timelineScrollX
                                                        when: !laneFlick.movingHorizontally
                                                    }

                                                    Rectangle {
                                                        width: chartViewport.chartWidth
                                                        height: laneFlick.height
                                                        color: "#fdf9f3"
                                                        border.color: "#e1d7c9"
                                                        radius: 10

                                                        Repeater {
                                                            model: root.tickCount
                                                            delegate: Rectangle {
                                                                x: chartViewport.leadingGap + index * chartViewport.tickPixels
                                                                y: 0
                                                                width: index % 5 === 0 ? 2 : 1
                                                                height: parent.height
                                                                color: index % 5 === 0 ? "#d0bca0" : "#efe5d5"
                                                            }
                                                        }

                                                        Repeater {
                                                            model: modelData.entries
                                                            delegate: Item {
                                                                id: entryItem
                                                                property bool isSpan: modelData.entryType === "span"
                                                                property real startX: chartViewport.leadingGap + modelData.startMs * chartViewport.pixelsPerMs
                                                                property real barWidth: isSpan ? Math.max(8, (modelData.endMs - modelData.startMs) * chartViewport.pixelsPerMs) : 16
                                                                property bool selected: root.selectedTimelineKey === root.timelineEntryKey(modelData)
                                                                x: startX
                                                                y: 10 + (modelData.stackIndex || 0) * 22 + (isSpan ? 0 : 2)
                                                                width: barWidth
                                                                height: isSpan ? 28 : 20

                                                                Rectangle {
                                                                    anchors.fill: parent
                                                                    radius: entryItem.isSpan ? 8 : 10
                                                                    color: entryItem.selected ? Qt.lighter(modelData.color, 1.15) : modelData.color
                                                                    border.color: entryItem.selected ? "#1f2428" : Qt.darker(modelData.color, 1.2)
                                                                    border.width: entryItem.selected ? 2 : 1
                                                                }

                                                                Rectangle {
                                                                    visible: !entryItem.isSpan
                                                                    anchors.horizontalCenter: parent.horizontalCenter
                                                                    anchors.verticalCenter: parent.verticalCenter
                                                                    width: 4
                                                                    height: 30
                                                                    color: Qt.darker(modelData.color, 1.15)
                                                                }

                                                                Rectangle {
                                                                    visible: !entryItem.isSpan
                                                                    anchors.centerIn: parent
                                                                    width: 12
                                                                    height: 12
                                                                    rotation: 45
                                                                    color: "#fff7ea"
                                                                    border.color: Qt.darker(modelData.color, 1.4)
                                                                }

                                                                Text {
                                                                    anchors.centerIn: parent
                                                                    width: parent.width - 6
                                                                    visible: entryItem.isSpan && parent.width >= 70
                                                                    text: modelData.label
                                                                    color: "white"
                                                                    font.pixelSize: root.smallFontPx
                                                                    elide: Text.ElideRight
                                                                }

                                                                Text {
                                                                    visible: !entryItem.isSpan
                                                                    x: parent.width + 4
                                                                    y: 1
                                                                    text: modelData.label
                                                                    color: root.bodyText
                                                                    font.pixelSize: root.smallFontPx
                                                                    elide: Text.ElideRight
                                                                    width: Math.max(80, laneFlick.width - parent.x - parent.width - 8)
                                                                }

                                                                MouseArea {
                                                                    id: entryMouse
                                                                    anchors.fill: parent
                                                                    hoverEnabled: true
                                                                    onClicked: {
                                                                        root.selectedTimelineKey = root.timelineEntryKey(modelData)
                                                                        toolsInsideBrowser.inspectTimelineEntry(modelData)
                                                                    }
                                                                }

                                                                ToolTip.visible: entryMouse.containsMouse
                                                                ToolTip.delay: 150
                                                                ToolTip.text: modelData.label
                                                                               + "\nStart: T+" + modelData.startMs + "ms"
                                                                               + (entryItem.isSpan ? ("\nEnd: T+" + modelData.endMs + "ms") : "")
                                                                               + (modelData.status ? ("\nStatus: " + modelData.status) : "")
                                                                               + (modelData.detail ? ("\n" + modelData.detail) : "")
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            SplitView.fillWidth: true
                            SplitView.fillHeight: true
                            SplitView.minimumHeight: 280
                            radius: 16
                            color: root.panelAltBg
                            border.color: root.panelBorder

                            SplitView {
                                anchors.fill: parent
                                anchors.margins: 12
                                orientation: Qt.Horizontal
                                handle: Item {
                                    implicitWidth: parent && parent.orientation === Qt.Horizontal ? 12 : parent ? parent.width : 12
                                    implicitHeight: parent && parent.orientation === Qt.Vertical ? 12 : parent ? parent.height : 12
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: parent.width > parent.height ? 56 : 4
                                        height: parent.width > parent.height ? 4 : 56
                                        radius: 2
                                        color: "#a9967a"
                                    }
                                }

                                Rectangle {
                                    SplitView.preferredWidth: 320
                                    SplitView.minimumWidth: 280
                                    SplitView.fillHeight: true
                                    radius: 12
                                    color: root.panelAltBg
                                    border.color: root.panelBorder

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 8

                                        TabBar {
                                            id: detailTabs
                                            Layout.fillWidth: true
                                            TabButton { text: root.uiText("Tool Calls", "工具调用") + " (" + toolsInsideBrowser.toolCalls.length + ")" }
                                            TabButton { text: root.uiText("Support Links", "支撑关系") + " (" + toolsInsideBrowser.supportLinks.length + ")" }
                                            TabButton { text: root.uiText("Artifacts", "工件") + " (" + toolsInsideBrowser.artifacts.length + ")" }
                                        }

                                        StackLayout {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            currentIndex: detailTabs.currentIndex

                                            ListView {
                                                clip: true
                                                model: toolsInsideBrowser.toolCalls
                                                delegate: Rectangle {
                                                    width: ListView.view.width
                                                    height: 54
                                                    radius: 8
                                                    color: modelData.toolCallId === root.selectedToolCallId ? "#e5efe9" : "#fcf8f1"
                                                    border.color: modelData.toolCallId === root.selectedToolCallId ? "#7fa393" : "#ddd1be"
                                                    MouseArea {
                                                        anchors.fill: parent
                                                        onClicked: {
                                                            root.selectedToolCallId = modelData.toolCallId
                                                            root.selectedSupportLinkId = ""
                                                            root.selectedArtifactId = ""
                                                            toolsInsideBrowser.inspectToolCall(modelData)
                                                        }
                                                    }
                                                    Column {
                                                        anchors.fill: parent
                                                        anchors.margins: 8
                                                        spacing: 2
                                                        Text {
                                                            text: modelData.toolId + " | " + modelData.status
                                                            color: root.bodyText
                                                            elide: Text.ElideRight
                                                            width: parent.width
                                                            font.bold: true
                                                        }
                                                        Text {
                                                            text: root.uiText("Round ", "轮次 ") + modelData.roundIndex + " | " + modelData.requestId
                                                            color: root.mutedText
                                                            elide: Text.ElideMiddle
                                                            width: parent.width
                                                            font.pixelSize: root.smallFontPx
                                                        }
                                                    }
                                                }
                                            }

                                            ListView {
                                                clip: true
                                                model: toolsInsideBrowser.supportLinks
                                                delegate: Rectangle {
                                                    width: ListView.view.width
                                                    height: 54
                                                    radius: 8
                                                    color: modelData.supportLinkId === root.selectedSupportLinkId ? "#e7edf5" : "#fcf8f1"
                                                    border.color: modelData.supportLinkId === root.selectedSupportLinkId ? "#7c96b8" : "#ddd1be"
                                                    MouseArea {
                                                        anchors.fill: parent
                                                        onClicked: {
                                                            root.selectedSupportLinkId = modelData.supportLinkId
                                                            root.selectedToolCallId = ""
                                                            root.selectedArtifactId = ""
                                                            toolsInsideBrowser.inspectSupportLink(modelData)
                                                        }
                                                    }
                                                    Column {
                                                        anchors.fill: parent
                                                        anchors.margins: 8
                                                        spacing: 2
                                                        Text {
                                                            text: modelData.supportType + " | " + modelData.source
                                                            color: root.bodyText
                                                            elide: Text.ElideRight
                                                            width: parent.width
                                                            font.bold: true
                                                        }
                                                        Text {
                                                            text: modelData.toolCallId + " -> " + modelData.targetId
                                                            color: root.mutedText
                                                            elide: Text.ElideMiddle
                                                            width: parent.width
                                                            font.pixelSize: root.smallFontPx
                                                        }
                                                    }
                                                }
                                            }

                                            ListView {
                                                clip: true
                                                model: toolsInsideBrowser.artifacts
                                                delegate: Rectangle {
                                                    width: ListView.view.width
                                                    height: 54
                                                    radius: 8
                                                    color: modelData.artifactId === root.selectedArtifactId ? "#f0e5d7" : "#fcf8f1"
                                                    border.color: modelData.artifactId === root.selectedArtifactId ? "#bc8d53" : "#ddd1be"
                                                    MouseArea {
                                                        anchors.fill: parent
                                                        onClicked: {
                                                            root.selectedArtifactId = modelData.artifactId
                                                            root.selectedToolCallId = ""
                                                            root.selectedSupportLinkId = ""
                                                            toolsInsideBrowser.inspectArtifact(modelData)
                                                        }
                                                    }
                                                    Column {
                                                        anchors.fill: parent
                                                        anchors.margins: 8
                                                        spacing: 2
                                                        Text {
                                                            text: modelData.kind
                                                            color: root.bodyText
                                                            elide: Text.ElideRight
                                                            width: parent.width
                                                            font.bold: true
                                                        }
                                                        Text {
                                                            text: modelData.relativePath
                                                            color: root.mutedText
                                                            elide: Text.ElideMiddle
                                                            width: parent.width
                                                            font.pixelSize: root.smallFontPx
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    SplitView.fillWidth: true
                                    SplitView.fillHeight: true
                                    SplitView.minimumWidth: 560
                                    radius: 12
                                    color: root.panelBg
                                    border.color: root.panelBorder

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 8

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Label { text: root.uiText("Inspector", "检查器"); font.bold: true; color: root.headerText }
                                            Item { Layout.fillWidth: true }
                                            Button {
                                                text: root.uiText("Clear", "清除")
                                                onClicked: {
                                                    root.selectedTimelineKey = ""
                                                    root.selectedToolCallId = ""
                                                    root.selectedSupportLinkId = ""
                                                    root.selectedArtifactId = ""
                                                    toolsInsideBrowser.clearInspector()
                                                }
                                            }
                                        }

                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 132
                                            radius: 10
                                            color: root.panelAltBg
                                            border.color: root.panelBorder
                                            GridLayout {
                                                anchors.fill: parent
                                                anchors.margins: 8
                                                columns: 1
                                                rowSpacing: 4
                                                Text {
                                                    Layout.fillWidth: true
                                                    text: toolsInsideBrowser.inspector.title || root.uiText("No selection", "未选择对象")
                                                    font.bold: true
                                                    font.pixelSize: Math.round(16 * root.uiScale)
                                                    color: root.headerText
                                                    wrapMode: Text.Wrap
                                                }
                                                Text {
                                                    Layout.fillWidth: true
                                                    text: (toolsInsideBrowser.inspector.kind || "")
                                                          + (toolsInsideBrowser.inspector.sourceType ? (" | " + toolsInsideBrowser.inspector.sourceType) : "")
                                                          + (toolsInsideBrowser.inspector.sourceId ? (" | " + toolsInsideBrowser.inspector.sourceId) : "")
                                                    color: root.mutedText
                                                    wrapMode: Text.WrapAnywhere
                                                }
                                                Text {
                                                    Layout.fillWidth: true
                                                    text: toolsInsideBrowser.inspector.summary || root.uiText("Click a swimlane node, tool call, support link, or artifact.", "点击泳道节点、工具调用、支撑关系或工件以查看详情。")
                                                    color: root.bodyText
                                                    wrapMode: Text.Wrap
                                                }
                                            }
                                        }

                                        Label { text: root.uiText("Content", "内容"); font.bold: true; color: root.bodyText }
                                        ScrollView {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            clip: true

                                            background: Rectangle {
                                                radius: 10
                                                color: "#fffdf9"
                                                border.color: "#ded2c1"
                                            }

                                            TextArea {
                                                readOnly: true
                                                wrapMode: TextArea.NoWrap
                                                text: toolsInsideBrowser.inspector.content || ""
                                                font.family: Qt.platform.os === "windows" ? "Consolas" : "Monospace"
                                                font.pixelSize: root.bodyFontPx
                                                selectByMouse: true
                                                leftPadding: 12
                                                rightPadding: 12
                                                topPadding: 12
                                                bottomPadding: 12
                                                background: null
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    footer: Rectangle {
        height: 38
        color: root.headerBarBg
        border.color: root.panelBorder
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            Text {
                text: toolsInsideBrowser.statusText
                color: root.mutedText
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                text: root.uiText("Selected trace: ", "当前 Trace：") + (toolsInsideBrowser.selectedTraceId || "-")
                color: root.bodyText
                elide: Text.ElideMiddle
                Layout.preferredWidth: 380
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
