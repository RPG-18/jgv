#include <QtGui/QFontDatabase>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#include "components/helpers.h"
#include "services/registry.h"

void messageHandler(QtMsgType level, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    auto logLevel = services::LogService::Level::Info;

    switch (level) {
    case QtWarningMsg: {
        logLevel = services::LogService::Level::Warning;
    } break;
    case QtCriticalMsg: {
        logLevel = services::LogService::Level::Critical;
    } break;
    case QtFatalMsg: {
        logLevel = services::LogService::Level::Fatal;
    } break;
    case QtInfoMsg: {
        logLevel = services::LogService::Level::Info;
    } break;
    default: {
        // skip debug messages
        return;
    }
    }
    Services->logger()->addMessage(logLevel, msg);
}

void loadFonts()
{
    const std::array<QString, 14> fonts = {":/fonts/RobotoMono-Bold.ttf",
                                           ":/fonts/RobotoMono-BoldItalic.ttf",
                                           ":/fonts/RobotoMono-ExtraLight.ttf",
                                           ":/fonts/RobotoMono-ExtraLightItalic.ttf",
                                           ":/fonts/RobotoMono-Italic.ttf",
                                           ":/fonts/RobotoMono-Light.ttf"
                                           ":/fonts/RobotoMono-LightItalic.ttf",
                                           ":/fonts/RobotoMono-Medium.ttf",
                                           ":/fonts/RobotoMono-MediumItalic.ttf",
                                           ":/fonts/RobotoMono-Regular.ttf",
                                           ":/fonts/RobotoMono-SemiBold.ttf",
                                           ":/fonts/RobotoMono-SemiBoldItalic.ttf",
                                           ":/fonts/RobotoMono-Thin.ttf",
                                           ":/fonts/RobotoMono-ThinItalic.ttf"};
    for (const auto &font : fonts) {
        const auto res = QFontDatabase::addApplicationFont(font);
        if (res == -1) {
            qDebug() << "failed load font" << font;
        }
    }
}

int main(int argc, char *argv[])
{
    Services->setLogger(std::make_shared<services::LogService>());

    components::registerTypes();
    //qInstallMessageHandler(messageHandler);

    QCoreApplication::setApplicationName("jaeger-graph-view");
    QCoreApplication::setOrganizationName("jaeger-graph-view");

    QGuiApplication app(argc, argv);
    loadFonts();

    QQmlApplicationEngine engine;
    const QUrl url("qrc:/qml/main.qml");

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
