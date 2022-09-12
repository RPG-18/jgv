#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVector>

namespace trace {

struct Tag
{
    QString key;
    QVariant value;
};

using Tags = QVector<Tag>;
} // namespace trace