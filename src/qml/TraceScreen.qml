import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: page
    property var trace

    SplitView {
        anchors.fill: parent
        orientation: Qt.Vertical

        ServiceMap {
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            graph: page.trace
        }

        FlatLogs {
            SplitView.fillWidth: true
            SplitView.minimumHeight: 300
            graph: page.trace
        }
    }
}
