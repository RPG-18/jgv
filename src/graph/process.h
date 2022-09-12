#pragma once

#include <QtCore/QString>

#include "tag.h"

namespace graph {

struct Process
{
    QString name;
    Tags tags;
};
} // namespace graph