import QtQuick
import QtQuick.Controls

Dialog {
    id: dialog

    width: 300
    height: 150

    function show(e) {
        label.text = e;
        dialog.open();
    }

    title: qsTr("Error")
    modal: true
    standardButtons: Dialog.Ok

    Text {
        id: label

        anchors.fill: parent
        wrapMode: Text.WordWrap
    }
}
