import QtQuick 6.0
import QtQuick.Controls 6.0

ApplicationWindow {
    id: appWindow
    width: 1920
    height: 1080
    visible: true
    title: qsTr("Jaeger Graph View")

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuSeparator {
            }
            Action {
                text: qsTr("&Quit")
                onTriggered: Qt.quit()
            }
        }
    }

    header: ToolBar {
    }

    StackView {
        id: stackView

        anchors.fill: parent
        initialItem: startPage
    }

    Component {
        id: startPage

        DownloadTraceScreen {
        }
    }
}
