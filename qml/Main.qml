import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    visible: true
    width: Screen.width
    height: Screen.height
    visibility: Window.FullScreen      // <-- makes it fullscreen
    title: "Advanced Powder Coating Oven"

  // 'oven' is the C++ object we injected from main.cpp:
  // engine.rootContext()->setContextProperty("oven", &backend);
  // It exposes QML-callable methods (start/stop) and a 'status' property.

  Column {
    anchors.centerIn: parent                     // center the column in the window
    spacing: 24                                  // vertical gap between children

    Text {
      // Bind the text to oven.status, so any statusChanged() signal updates the UI automatically.
      text: "Status: " + (oven ? oven.status : "…")
      font.pixelSize: 28                         // big enough to read on the panel
      horizontalAlignment: Text.AlignHCenter     // center the caption
    }

    Row {
      spacing: 16                                // gap between buttons

      Button {
        text: "Start"                            // label for start action
        onClicked: oven.start()                  // call into C++ (OvenBackend::start)
      }

      Button {
        text: "Stop"                             // label for stop action
        onClicked: oven.stop()                   // call into C++ (OvenBackend::stop)
      }
    }
  }
}
