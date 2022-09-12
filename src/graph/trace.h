#pragma once

#include <memory>

#include "trace/trace.h"

#include "process.h"
#include "span.h"

namespace graph {

struct Trace
{
    QString traceID;
    Span *root;

    std::vector<std::unique_ptr<Span>> spans;
    std::vector<std::unique_ptr<Process>> process;
};

struct TraceGraph
{
    std::vector<std::shared_ptr<Trace>> traces;

    static std::shared_ptr<TraceGraph> makeGraph(const trace::TraceDocument &document);
};

} // namespace graph
