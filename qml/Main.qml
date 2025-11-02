import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    visibility: Window.FullScreen
    title: "Oven Controller"

    function fmt(x) { return isNaN(x) ? "N/A" : x.toFixed(1) + " °C" }

    // Popup numpad for temperature entry
    Popup {
        id: numpadPopup
        width: 400
        height: 500
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        anchors.centerIn: parent

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

            // Display for entered value
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                color: "white"
                border.color: "#2196F3"
                border.width: 3
                radius: 8

                Text {
                    id: numpadDisplay
                    anchors.centerIn: parent
                    text: tempInput.text || "0"
                    font.pixelSize: 36
                    font.bold: true
                    color: "#333"
                }
            }

            // Hidden text input to store the value
            TextInput {
                id: tempInput
                visible: false
                text: "0"
                validator: IntValidator { bottom: 0; top: 400 }
            }

            // Numpad grid
            GridLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 3
                rowSpacing: 10
                columnSpacing: 10

                // Number buttons 1-9
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

                // Clear button
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

                // Zero button
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

                // Backspace button
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

            // Action buttons
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
                            oven.sendManualSetpoint(value)
                        }
                        numpadPopup.close()  // Always close immediately
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // Top status bar
        Label {
            text: "State: " + oven.status
            font.pixelSize: 28
            font.bold: true
            color: "#333"
            Layout.alignment: Qt.AlignHCenter
        }

        // Main content area
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 15

            // Left side: Temperature channels
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

            // Right side: Controls
            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 15

                // State control buttons
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

                // Temperature setpoint control
                GroupBox {
                    title: "Setpoint Control"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    font.pixelSize: 20

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 10

                        // Temperature display and input
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
                                    tempInput.text = "0"
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

                        // Big increment/decrement buttons
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

                        // Status message
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
}