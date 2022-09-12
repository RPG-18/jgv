#pragma once

#include <QtCore/QAbstractListModel>

#include "graph/tag.h"

namespace components {

class TagModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles { Key = Qt::UserRole + 1, Value };

    TagModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTags(const graph::Tags &tags);

private:
    graph::Tags m_tags;
};

} // namespace components