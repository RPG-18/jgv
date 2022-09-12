#include "tag_model.h"

namespace components {

TagModel::TagModel(QObject *parent)
    : QAbstractListModel(parent)
{}

void TagModel::setTags(const graph::Tags &tags)
{
    beginResetModel();
    m_tags = tags;
    endResetModel();
}

int TagModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_tags.size();
}

QVariant TagModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= m_tags.size()) {
        return {};
    }

    const auto row = index.row();
    switch (role) {
    case Key:
        return m_tags[row].key;
    case Value:
        return m_tags[row].value;
    default:
        return {};
    }
};

QHash<int, QByteArray> TagModel::roleNames() const
{
    static QHash<int, QByteArray> roles{
        {Key, "key"},
        {Value, "value"},
    };
    return roles;
}

} // namespace components