import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import jaeger
import "components" as Components
import "pages.js" as Pages

Item {
    TraceDownloader {
        id: downloader
        onErrorDownload: errMessage => {
            errDialog.show(errMessage);
        }

        onDownloaded: graph => {
            Pages.createTraceScreen(graph);
            traceUrl.text = "";
        }
    }

    Components.ErrorDialog {
        id: errDialog
        anchors.centerIn: parent
    }

    RowLayout {
        anchors.centerIn: parent

        TextField {
            id: traceUrl
            placeholderText: qsTr("http://localhost:16686/api/traces/7ae7749cafeeb4a0")
            Layout.minimumWidth: 450
        }

        Button {
            text: "GO"
            onClicked: {
                if (traceUrl.text.length !== 0) {
                    downloader.download(traceUrl.text);
                }
            }
        }
    }
}
