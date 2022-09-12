#pragma once

#include <QtCore/QObject>

#include "graph/trace.h"

namespace components {
struct TraceGraph
{
    Q_GADGET

public:
    std::shared_ptr<graph::TraceGraph> data;
};
} // namespace components

Q_DECLARE_METATYPE(components::TraceGraph);
