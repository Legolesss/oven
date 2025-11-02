import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    visibility: Window.FullScreen
    title: "Oven Controller"

    function fmt(x) { return isNaN(x) ? "N/A" : x.toFixed(1) + " °C" }

    // Cure Complete Popup
    Popup {
        id: cureCompletePopup
        width: 500
        height: 300
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose
        anchors.centerIn: parent
        visible: oven.autoCureComplete

        background: Rectangle {
            color: "#4CAF50"
            border.color: "#2E7D32"
            border.width: 3
            radius: 15
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 30
            spacing: 20

            Label {
                text: "✓ CURE COMPLETE"
                font.pixelSize: 42
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                text: "The curing cycle has finished successfully."
                font.pixelSize: 22
                color: "white"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                text: "OK"
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 200
                Layout.preferredHeight: 80
                font.pixelSize: 28
                font.bold: true

                background: Rectangle {
                    color: parent.pressed ? "#1565C0" : "white"
                    radius: 10
                }

                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: parent.pressed ? "white" : "#4CAF50"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    oven.acknowledgeAutoCureComplete()
                }
            }
        }
    }

    // Numpad Popup
    Popup {
        id: numpadPopup
        width: 400
        height: 500
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        anchors.centerIn: parent

        property bool isAutoMode: false

        background: Rectangle {
            color: "#f5f5f5"
            border.color: "#333"
            border.width: 2
            radius: 10
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Label {
                text: "Enter Temperature (°C)"
                font.pixelSize: 24
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                color: "white"
                border.color: "#2196F3"
                border.width: 3
                radius: 8

                Text {
                    anchors.centerIn: parent
                    text: tempInput.text || "0"
                    font.pixelSize: 36
                    font.bold: true
                    color: "#333"
                }
            }

            TextInput {
                id: tempInput
                visible: false
                text: "200"
                validator: IntValidator { bottom: 0; top: 400 }
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 3
                rowSpacing: 10
                columnSpacing: 10

                Repeater {
                    model: 9
                    Button {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: (index + 1).toString()
                        font.pixelSize: 28
                        font.bold: true

                        background: Rectangle {
                            color: parent.pressed ? "#2196F3" : "#fff"
                            border.color: "#2196F3"
                            border.width: 2
                            radius: 8
                        }

                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            color: parent.pressed ? "white" : "#333"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (tempInput.text === "0") {
                                tempInput.text = text
                            } else if (tempInput.text.length < 3) {
                                tempInput.text += text
                            }
                        }
                    }
                }

                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: "C"
                    font.pixelSize: 28
                    font.bold: true

                    background: Rectangle {
                        color: parent.pressed ? "#f44336" : "#fff"
                        border.color: "#f44336"
                        border.width: 2
                        radius: 8
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: parent.pressed ? "white" : "#f44336"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: tempInput.text = "0"
                }

                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: "0"
                    font.pixelSize: 28
                    font.bold: true

                    background: Rectangle {
                        color: parent.pressed ? "#2196F3" : "#fff"
                        border.color: "#2196F3"
                        border.width: 2
                        radius: 8
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: parent.pressed ? "white" : "#333"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (tempInput.text === "0") {
                            return
                        } else if (tempInput.text.length < 3) {
                            tempInput.text += "0"
                        }
                    }
                }

                Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: "⌫"
                    font.pixelSize: 28
                    font.bold: true

                    background: Rectangle {
                        color: parent.pressed ? "#FF9800" : "#fff"
                        border.color: "#FF9800"
                        border.width: 2
                        radius: 8
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: parent.pressed ? "white" : "#FF9800"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        if (tempInput.text.length > 1) {
                            tempInput.text = tempInput.text.slice(0, -1)
                        } else {
                            tempInput.text = "0"
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 15

                Button {
                    text: "Cancel"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    font.pixelSize: 22

                    background: Rectangle {
                        color: parent.pressed ? "#ccc" : "#e0e0e0"
                        radius: 8
                    }

                    onClicked: numpadPopup.close()
                }

                Button {
                    text: "Set"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    font.pixelSize: 22
                    font.bold: true

                    background: Rectangle {
                        color: parent.pressed ? "#1976D2" : "#4CAF50"
                        radius: 8
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        let value = parseInt(tempInput.text)
                        if (value >= 0 && value <= 400) {
                            if (numpadPopup.isAutoMode) {
                                // For auto mode, just close - value will be used when Start is clicked
                            } else {
                                oven.sendManualSetpoint(value)
                            }
                        }
                        numpadPopup.close()
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // Top bar with mode selector
        RowLayout {
            Layout.fillWidth: true
            spacing: 15

            Button {
                text: "MANUAL MODE"
                Layout.preferredWidth: parent.width / 2 - 10
                Layout.preferredHeight: 60
                font.pixelSize: 22
                font.bold: true
                enabled: !oven.autoModeActive

                background: Rectangle {
                    color: !stackLayout.currentIndex ? "#4CAF50" : (parent.pressed ? "#ccc" : "#e0e0e0")
                    radius: 8
                }

                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: !stackLayout.currentIndex ? "white" : "#333"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: stackLayout.currentIndex = 0
            }

            Button {
                text: "AUTO MODE"
                Layout.preferredWidth: parent.width / 2 - 10
                Layout.preferredHeight: 60
                font.pixelSize: 22
                font.bold: true

                background: Rectangle {
                    color: stackLayout.currentIndex ? "#FF9800" : (parent.pressed ? "#ccc" : "#e0e0e0")
                    radius: 8
                }

                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: stackLayout.currentIndex ? "white" : "#333"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: stackLayout.currentIndex = 1
            }
        }

        // Status bar
        Label {
            text: "State: " + oven.status
            font.pixelSize: 28
            font.bold: true
            color: "#333"
            Layout.alignment: Qt.AlignHCenter
        }

        // Stack layout for Manual/Auto screens
        StackLayout {
            id: stackLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 0

            // ========== MANUAL MODE SCREEN ==========
            Item {
                RowLayout {
                    anchors.fill: parent
                    spacing: 15

                    GroupBox {
                        title: "THKA Channels"
                        Layout.fillHeight: true
                        Layout.preferredWidth: 320
                        font.pixelSize: 20

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 12

                            Repeater {
                                model: oven.thkaTemps.length
                                delegate: RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 15

                                    Label {
                                        text: "CH" + (index + 1) + ":"
                                        color: "#333"
                                        font.pixelSize: 20
                                        font.bold: true
                                        Layout.preferredWidth: 60
                                    }
                                    Label {
                                        text: fmt(oven.thkaTemps[index])
                                        color: "#2196F3"
                                        font.pixelSize: 20
                                        font.bold: true
                                        Layout.fillWidth: true
                                    }
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        spacing: 15

                        GroupBox {
                            title: "State Control"
                            Layout.fillWidth: true
                            font.pixelSize: 20

                            GridLayout {
                                anchors.fill: parent
                                anchors.margins: 6
                                columns: 3
                                rowSpacing: 8
                                columnSpacing: 8

                                Button {
                                    text: "Idle"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 55
                                    font.pixelSize: 18
                                    font.bold: true
                                    onClicked: oven.enterIdle()

                                    background: Rectangle {
                                        color: parent.pressed ? "#888" : "#9E9E9E"
                                        radius: 6
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        font: parent.font
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Button {
                                    text: "Warming"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 55
                                    font.pixelSize: 18
                                    font.bold: true
                                    onClicked: oven.enterWarming()

                                    background: Rectangle {
                                        color: parent.pressed ? "#E65100" : "#FF9800"
                                        radius: 6
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        font: parent.font
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Button {
                                    text: "Ready"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 55
                                    font.pixelSize: 18
                                    font.bold: true
                                    onClicked: oven.enterReady()

                                    background: Rectangle {
                                        color: parent.pressed ? "#1976D2" : "#2196F3"
                                        radius: 6
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        font: parent.font
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Button {
                                    text: "Curing"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 55
                                    font.pixelSize: 18
                                    font.bold: true
                                    onClicked: oven.enterCuring()

                                    background: Rectangle {
                                        color: parent.pressed ? "#388E3C" : "#4CAF50"
                                        radius: 6
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        font: parent.font
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Button {
                                    text: "Shutdown"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 55
                                    font.pixelSize: 18
                                    font.bold: true
                                    onClicked: oven.enterShutdown()

                                    background: Rectangle {
                                        color: parent.pressed ? "#5D4037" : "#795548"
                                        radius: 6
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        font: parent.font
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Button {
                                    text: "Fault"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 55
                                    font.pixelSize: 18
                                    font.bold: true
                                    onClicked: oven.enterFault()

                                    background: Rectangle {
                                        color: parent.pressed ? "#C62828" : "#f44336"
                                        radius: 6
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        font: parent.font
                                        color: "white"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }
                        }

                        GroupBox {
                            title: "Setpoint Control"
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            font.pixelSize: 20

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 10

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 90
                                    color: "white"
                                    border.color: "#2196F3"
                                    border.width: 3
                                    radius: 8

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            tempInput.text = oven.manualSetpoint.toFixed(0)
                                            numpadPopup.isAutoMode = false
                                            numpadPopup.open()
                                        }
                                    }

                                    ColumnLayout {
                                        anchors.centerIn: parent
                                        spacing: 2

                                        Label {
                                            text: oven.manualSetpoint.toFixed(0) + " °C"
                                            font.pixelSize: 38
                                            font.bold: true
                                            color: "#2196F3"
                                            Layout.alignment: Qt.AlignHCenter
                                        }

                                        Label {
                                            text: "Tap to enter"
                                            font.pixelSize: 14
                                            color: "#999"
                                            Layout.alignment: Qt.AlignHCenter
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Button {
                                        text: "−"
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 85
                                        font.pixelSize: 42
                                        font.bold: true

                                        background: Rectangle {
                                            color: parent.pressed ? "#1976D2" : "#2196F3"
                                            radius: 8
                                        }

                                        contentItem: Text {
                                            text: parent.text
                                            font: parent.font
                                            color: "white"
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        onClicked: {
                                            let newVal = Math.max(0, oven.manualSetpoint - 5)
                                            oven.sendManualSetpoint(newVal)
                                        }
                                    }

                                    Button {
                                        text: "+"
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 85
                                        font.pixelSize: 42
                                        font.bold: true

                                        background: Rectangle {
                                            color: parent.pressed ? "#1976D2" : "#2196F3"
                                            radius: 8
                                        }

                                        contentItem: Text {
                                            text: parent.text
                                            font: parent.font
                                            color: "white"
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        onClicked: {
                                            let newVal = Math.min(400, oven.manualSetpoint + 5)
                                            oven.sendManualSetpoint(newVal)
                                        }
                                    }
                                }

                                Label {
                                    text: oven.manualSetpointStatus
                                    wrapMode: Text.WordWrap
                                    font.pixelSize: 16
                                    color: oven.manualSetpointStatus.indexOf("✓") >= 0 ? "#4CAF50" : "#333"
                                    Layout.fillWidth: true
                                    horizontalAlignment: Text.AlignHCenter
                                }
                            }
                        }
                    }
                }
            }

            // ========== AUTO MODE SCREEN ==========
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 30

                    // Auto status display
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100
                        color: "#FFF3E0"
                        border.color: "#FF9800"
                        border.width: 3
                        radius: 10

                        Label {
                            anchors.centerIn: parent
                            text: oven.autoStatus
                            font.pixelSize: 26
                            font.bold: true
                            color: "#E65100"
                        }
                    }

                    // Temperature selection
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 150
                        color: "white"
                        border.color: "#FF9800"
                        border.width: 3
                        radius: 10
                        visible: !oven.autoModeActive

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                tempInput.text = "200"
                                numpadPopup.isAutoMode = true
                                numpadPopup.open()
                            }
                        }

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 5

                            Label {
                                text: tempInput.text + " °C"
                                font.pixelSize: 56
                                font.bold: true
                                color: "#FF9800"
                                Layout.alignment: Qt.AlignHCenter
                            }

                            Label {
                                text: "Tap to set target temperature"
                                font.pixelSize: 18
                                color: "#999"
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                    }

                    // Timer display (only during curing)
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120
                        color: "#E8F5E9"
                        border.color: "#4CAF50"
                        border.width: 3
                        radius: 10
                        visible: oven.autoModeActive && oven.autoCureTimeLeft > 0

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 5

                            Label {
                                text: {
                                    let mins = Math.floor(oven.autoCureTimeLeft / 60)
                                    let secs = oven.autoCureTimeLeft % 60
                                    return mins + ":" + (secs < 10 ? "0" : "") + secs
                                }
                                font.pixelSize: 64
                                font.bold: true
                                color: "#2E7D32"
                                Layout.alignment: Qt.AlignHCenter
                            }

                            Label {
                                text: "Time Remaining"
                                font.pixelSize: 20
                                color: "#4CAF50"
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                    }

                    // Start/Cancel button
                    Button {
                        text: oven.autoModeActive ? "CANCEL AUTO MODE" : "START AUTO CYCLE"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120
                        font.pixelSize: 32
                        font.bold: true

                        background: Rectangle {
                            color: oven.autoModeActive
                                ? (parent.pressed ? "#C62828" : "#f44336")
                                : (parent.pressed ? "#388E3C" : "#4CAF50")
                            radius: 15
                        }

                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            if (oven.autoModeActive) {
                                oven.cancelAutoMode()
                            } else {
                                let targetTemp = parseInt(tempInput.text)
                                if (targetTemp >= 0 && targetTemp <= 400) {
                                    oven.startAutoMode(targetTemp)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}