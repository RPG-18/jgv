#include <algorithm>

#include "span_model.h"

namespace {
QString durationToString(std::chrono::microseconds duration)
{
    static const QVector<QString> postfix = {
        "µs",
        "ms",
        "sec",
    };

    double microseconds = duration.count();
    int count = 0;
    while (microseconds > 1000) {
        microseconds = microseconds / 1000;
        count++;
    }
    return QString("%1%2").arg(QString::number(microseconds, 'f', 2)).arg(postfix.value(count, ""));
}

} // namespace
namespace components {

SpanModel::SpanModel(QObject *parent)
    : QAbstractListModel(parent)
{}

void SpanModel::setSpans(QVector<graph::Span *> &&spans)
{
    beginResetModel();
    m_spans.swap(spans);
    std::sort(m_spans.begin(), m_spans.end(), [](graph::Span *a, graph::Span *b) {
        return a->startTime < b->startTime;
    });
    endResetModel();
}

int SpanModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_spans.size();
}

QVariant SpanModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= m_spans.size()) {
        return {};
    }

    const auto row = index.row();
    switch (role) {
    case SpanID:
        return m_spans[row]->spanID;
    case OperationName:
        return m_spans[row]->operationName;
    case Duration:
        return durationToString(m_spans[row]->duration);
    default:
        return {};
    }
}

QHash<int, QByteArray> SpanModel::roleNames() const
{
    static QHash<int, QByteArray> roles{{SpanID, "spanID"},
                                        {OperationName, "operationName"},
                                        {Duration, "duration"}};
    return roles;
}

} // namespace components
