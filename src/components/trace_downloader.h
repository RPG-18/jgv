#pragma once

#include <QtCore/QObject>

#include "trace.h"

class QNetworkReply;
class QNetworkAccessManager;

namespace components {

class TraceDownloader : public QObject
{
    Q_OBJECT
public:
    explicit TraceDownloader(QObject *parent = nullptr);

    Q_INVOKABLE void download(const QString &url);

signals:

    void errorDownload(const QString &message);
    void downloaded(TraceGraph traceGraph);

private slots:

    void onFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
};

} // namespace components
