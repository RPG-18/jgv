import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import jaeger
import "style.js" as Style

Item {
    id: item
    property var graph

    property var columnWidths: [40, 110, 170, 170, 400]

    function columnWidthProvider(column) {
        return columnWidths[column];
    }
    onWidthChanged: {
        let sumWidth = 0;
        for (var i = 0; i < columnWidths.length - 1; i++) {
            sumWidth += columnWidths[i];
        }
        columnWidths[item.columnWidths.length - 1] = item.width - sumWidth;
        view.forceLayout();
    }

    FlatLogModel {
        id: model
        graph: item.graph
    }

    ProcessModel {
        id: processModel
        graph: item.graph
    }

    LogLevelModel {
        id: logLevelModel
    }

    FilteredLogModel {
        id: filterModel
        sourceModel: model

        logLevel: logLevelModel
        process: processModel
    }

    Popup {
        id: tableFieldsPopup
        anchors.centerIn: parent

        modal: true
        focus: true
        closePolicy: Popup.CloseOnPressOutside

        FieldsModel {
            id: fieldModel
        }

        GridLayout {
            anchors.fill: parent
            columns: 2
            columnSpacing: 1
            rowSpacing: 1

            Repeater {
                model: fieldModel

                Rectangle {
                    Layout.column: 0
                    Layout.row: index
                    Layout.preferredHeight: 25
                    Layout.preferredWidth: 80
                    implicitWidth: keyTxt.width + 8
                    color: index % 2 === 0 ? "#ffffff" : "#efefef"

                    Text {
                        id: keyTxt
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 2
                        text: key
                    }
                }
            }

            Repeater {
                model: fieldModel

                Rectangle {
                    Layout.column: 1
                    Layout.row: index
                    Layout.preferredHeight: 25
                    Layout.fillWidth: true
                    Layout.maximumWidth: 400

                    implicitWidth: valueTxt.width + 8
                    color: index % 2 === 0 ? "#ffffff" : "#efefef"

                    Text {
                        id: valueTxt

                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 2

                        text: value
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Item {
                Layout.columnSpan: 2
                Layout.fillHeight: true
            }
        }

        function drawFields(f) {
            fieldModel.setFields(f);
            tableFieldsPopup.open();
        }
    }

    ListModel {
        id: headerModel

        ListElement {
            name: ""
        }

        ListElement {
            name: "Time"
        }

        ListElement {
            name: "Span"
        }

        ListElement {
            name: "Process"
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 0
        anchors.fill: parent

        Rectangle {
            height: 30
            Layout.fillWidth: true

            Row {
                Repeater {
                    model: headerModel
                    Rectangle {
                        id: root
                        width: columnWidths[index]
                        height: 30

                        Text {
                            anchors.centerIn: parent
                            text: model.name
                            font.bold: true
                        }

                        Button {
                            text: "⚙"
                            implicitHeight: 22
                            implicitWidth: 22
                            anchors.right: parent.right
                            anchors.rightMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            visible: index === 0 || index === 3

                            onClicked: {
                                if (index === 0) {
                                    logLevelPopup.open();
                                }
                                if (index === 3) {
                                    processPopup.open();
                                }
                            }

                            Popup {
                                id: logLevelPopup
                                closePolicy: Popup.CloseOnPressOutside
                                Column {
                                    Repeater {
                                        model: logLevelModel

                                        CheckBox {
                                            id: logLevelBox
                                            checked: isChecked
                                            text: level

                                            onCheckedChanged: {
                                                logLevelModel.checked(index, logLevelBox.checked);
                                            }
                                        }
                                    }
                                }
                            }

                            Popup {
                                id: processPopup
                                closePolicy: Popup.CloseOnPressOutside

                                Column {
                                    Repeater {
                                        model: processModel
                                        CheckBox {
                                            id: processBox
                                            checked: isChecked
                                            text: processName

                                            onCheckedChanged: {
                                                processModel.checked(index, processBox.checked);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            id: splitter
                            visible: index > 0

                            color: "#efefef"
                            height: 30
                            width: 2
                            x: columnWidths[index] - 1
                            onXChanged: {
                                if (drag.active) {
                                    item.columnWidths[index] = splitter.x;
                                    root.width = splitter.x + 1;
                                    view.forceLayout();
                                }
                            }

                            DragHandler {
                                id: drag

                                yAxis.enabled: false
                                xAxis.enabled: true
                                cursorShape: Qt.SizeHorCursor
                            }
                        }
                    }
                }
            }
        }

        TableView {
            id: view
            model: filterModel
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            boundsMovement: Flickable.StopAtBounds
            columnWidthProvider: item.columnWidthProvider

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ScrollBar.horizontal: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: Rectangle {
                implicitWidth: 100
                implicitHeight: 25
                color: hasError ? row % 2 === 0 ? "#F6CECE" : "#F6D8CE" : row % 2 === 0 ? "#ffffff" : "#efefef"

                StackLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 2
                    currentIndex: column

                    Text {
                        text: level
                        color: levelColor

                        font.bold: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        text: time

                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        // span
                        text: span

                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        font.family: Style.MonoFontFamily
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        // process
                        text: process

                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                    }

                    Item {

                        // message
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        clip: true

                        FieldsModel {
                            id: feilsdModel
                            fields: message
                        }

                        Row {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Button {
                                text: "⤢"
                                implicitHeight: 22
                                implicitWidth: 22
                                onClicked: tableFieldsPopup.drawFields(message)
                            }

                            Repeater {
                                model: feilsdModel
                                Row {
                                    spacing: 2
                                    Text {
                                        text: key
                                        color: "#7a7777"
                                    }

                                    Text {
                                        text: " = "
                                        color: "#7a7777"
                                    }

                                    Text {
                                        text: value
                                    }

                                    Text {
                                        text: "; "
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
