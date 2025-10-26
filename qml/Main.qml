import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    visibility: Window.FullScreen
    title: "Oven Controller"

    function fmt(x) { return isNaN(x) ? "N/A" : x.toFixed(1) + " °C" }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        Label {
            text: "State: " + oven.status
            font.pixelSize: 28
            color: "black"
            Layout.alignment: Qt.AlignHCenter
        }

        // ---- THKA live channels grid ----
        GroupBox {
            title: "THKA Channels"
            Layout.fillWidth: true

            GridLayout {
                id: grid
                columns: 2
                columnSpacing: 18
                rowSpacing: 10
                anchors.margins: 12

                Repeater {
                    model: oven.thkaTemps.length
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: "CH" + (index + 1) + ":"
                            color: "black"
                            font.pixelSize: 20
                            Layout.preferredWidth: 80
                        }
                        Label {
                            text: fmt(oven.thkaTemps[index])
                            color: "black"
                            font.pixelSize: 20
                        }
                    }
                }
            }
        }

        // ---- State control buttons (unchanged) ----
        Flow {
            width: parent.width
            spacing: 12
            Layout.alignment: Qt.AlignHCenter

            Button { text: "Idle";     onClicked: oven.enterIdle() }
            Button { text: "Warming";  onClicked: oven.enterWarming() }
            Button { text: "Ready";    onClicked: oven.enterReady() }
            Button { text: "Curing";   onClicked: oven.enterCuring() }
            Button { text: "Shutdown"; onClicked: oven.enterShutdown() }
            Button { text: "Fault";    onClicked: oven.enterFault() }
        }

        
        // ---- Manual temperature entry ----
        GroupBox {
            title: "Manual Setpoint"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.margins: 12
                spacing: 10

                Label {
                    // Explain how to use the scroll-friendly control below.
                    text: "Use the scroll arrows or mouse wheel to pick a target temperature, then press Send."
                    wrapMode: Text.WordWrap
                    font.pixelSize: 18
                    color: "black"
                }

                RowLayout {
                    spacing: 12

                    // Scrollable spin box for selecting the desired temperature in °C.
                    SpinBox {
                        id: setpointSpinner
                        from: 0
                        to: 400
                        value: oven.manualSetpoint
                        stepSize: 1
                        editable: true
                        Layout.preferredWidth: 180
                    }

                    // Button that forwards the selected temperature to the THKA controller via the backend.
                    Button {
                        text: "Send Setpoint"
                        onClicked: oven.sendManualSetpoint(setpointSpinner.value)
                    }
                }

                // Status readout that reflects success/failure of the last manual transmission.
                Label {
                    text: oven.manualSetpointStatus
                    wrapMode: Text.WordWrap
                    font.pixelSize: 16
                    color: "#444"
                }
            }
        }
    }   
}
