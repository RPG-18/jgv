#pragma once

#include <QtCore/QAbstractListModel>

#include "graph/span.h"

namespace components {
class SpanModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles { SpanID = Qt::UserRole + 1, OperationName, Duration };

    SpanModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setSpans(QVector<graph::Span *> &&spans);

private:
    QVector<graph::Span *> m_spans;
};
} // namespace components
