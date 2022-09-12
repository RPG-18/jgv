import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import jaeger
import "style.js" as Style

Item {
    id: item
    property var graph

    width: 900
    height: 900

    ServiceMapNodeItem {
        id: nodeItem
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        Flickable {
            topMargin: 80
            leftMargin: 80
            SplitView.fillWidth: true
            SplitView.fillHeight: true

            contentWidth: svcMap.width
            contentHeight: svcMap.height

            ServiceMap {
                id: svcMap

                visible: true

                //anchors.fill: parent
                graph: item.graph
                delegate: Rectangle {
                    implicitHeight: content.height + 10
                    implicitWidth: content.width + 10

                    visible: true
                    border.color: "black"
                    border.width: 1

                    ColumnLayout {
                        id: content
                        x: 5
                        y: 5

                        Text {
                            text: node.name
                            Layout.minimumWidth: 100
                            font.bold: true
                        }

                        Rectangle {
                            visible: node.hasEdges
                            height: 1
                            width: 10
                            Layout.fillWidth: true
                            color: "gray"
                        }

                        Repeater {
                            model: node.operations

                            Text {
                                text: modelData
                            }
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            nodeItem.setNode(node);
                        }
                    }
                }
            }
        }

        Rectangle {
            SplitView.fillHeight: true
            SplitView.preferredWidth: item.width * 0.25
            SplitView.minimumWidth: 200

            width: 200
            height: 100

            ColumnLayout {
                id: nodeContent

                anchors.fill: parent

                Text {
                    text: nodeItem.processName
                    font.bold: true
                    Layout.leftMargin: 8
                }
                Text {
                    Layout.leftMargin: 8
                    text: "Tags"
                }

                Rectangle {
                    height: 1
                    width: 10
                    Layout.fillWidth: true

                    color: "gray"
                }

                ListView {
                    id: tagView
                    width: 200
                    height: 200
                    clip: true
                    Layout.fillWidth: true

                    model: nodeItem.tagModel

                    delegate: Rectangle {
                        width: tagView.width
                        height: 48
                        color: index % 2 === 0 ? "#ffffff" : "#efefef"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 2
                            Text {
                                text: key
                                Layout.fillWidth: true
                                font: Style.MonoFontFamily
                            }

                            Text {
                                text: value
                                Layout.fillWidth: true
                                font: Style.MonoFontFamily
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }

                Text {
                    Layout.leftMargin: 8
                    text: "Spans"
                }

                Rectangle {
                    height: 1
                    width: 10
                    Layout.fillWidth: true

                    color: "gray"
                }

                ListView {
                    id: spanView
                    width: 200
                    height: 300
                    clip: true

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    model: nodeItem.spanModel

                    delegate: Rectangle {
                        width: spanView.width
                        height: 48
                        color: index % 2 === 0 ? "#ffffff" : "#efefef"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 2
                            Row {
                                spacing: 4
                                Text {
                                    text: "spanID:"
                                    color: "#7a7777"
                                }

                                Text {
                                    text: spanID
                                    font: Style.MonoFontFamily
                                }

                                Text {
                                    text: "|"
                                    color: "#7a7777"
                                }

                                Text {
                                    text: "duration:"
                                    color: "#7a7777"
                                }

                                Text {
                                    text: duration
                                }
                            }

                            Text {
                                text: operationName
                            }
                        }
                    }
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }
            }
        }
    }
}
