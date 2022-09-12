#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "graph/trace.h"

#include "trace_downloader.h"

namespace components {

TraceDownloader::TraceDownloader(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    QObject::connect(m_manager,
                     &QNetworkAccessManager::finished,
                     this,
                     &TraceDownloader::onFinished);
}

void TraceDownloader::download(const QString &url)
{
    m_manager->get(QNetworkRequest(url));
}

void TraceDownloader::onFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "failed download trace" << reply->url() << reply->errorString();
        emit errorDownload(reply->errorString());
        return;
    }

    auto data = reply->readAll();
    trace::TraceParseError parseError;
    auto traceDoc = trace::TraceDocument::parseDocument(data, &parseError);
    if (parseError.error != trace::TraceParseError::ParseError::NoError) {
        qWarning() << "failed parse trace" << reply->url();
        emit errorDownload(parseError.errorString());
        return;
    }

    auto traceGraph = graph::TraceGraph::makeGraph(traceDoc);
    components::TraceGraph result;
    result.data = traceGraph;
    emit downloaded(result);
}

} // namespace components