function createTraceScreen(trace) {
    let component = Qt.createComponent("qrc:/qml/TraceScreen.qml");

    let traceScreen = component.createObject(appWindow);
    stackView.push(traceScreen, {"trace": trace});
}
