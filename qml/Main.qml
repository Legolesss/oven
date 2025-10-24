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
    }
}
