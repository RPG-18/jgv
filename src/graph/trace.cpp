#include "trace.h"

namespace {

graph::Tags copyTags(const trace::Tags &tags)
{
    graph::Tags tgs;
    tgs.reserve(tags.size());
    for (const auto &tag : tags) {
        graph::Tag gTag;
        gTag.key = tag.key;
        gTag.value = tag.value;
        tgs.emplace_back(gTag);
    }

    return tgs;
}

graph::LogRecord::Level logLevel(const QString &level)
{
    static const QHash<QString, graph::LogRecord::Level> levels = {
        {"debug", graph::LogRecord::Level::Debug},
        {"info", graph::LogRecord::Level::Info},
        {"warn", graph::LogRecord::Level::Warn},
        {"warning", graph::LogRecord::Level::Warn},
        {"error", graph::LogRecord::Level::Error},
        {"panic", graph::LogRecord::Level::Panic},
        {"fatal", graph::LogRecord::Level::Fatal},
    };

    return levels.value(level, graph::LogRecord::Level::No);
};

QVector<graph::LogRecord> copyLogs(const QVector<trace::LogRecord> &logs)
{
    QVector<graph::LogRecord> lgs;
    lgs.reserve(logs.size());

    for (const auto &record : logs) {
        graph::LogRecord rec;
        rec.timestamp = record.timestamp;
        rec.fields.reserve(record.fields.size());

        for (const auto &field : record.fields) {
            if (field.key != "level") {
                graph::LogRecord::Field f;
                f.key = field.key;
                f.value = field.value;
                rec.fields.emplace_back(f);
                if (field.key == "error") {
                    rec.hasError = true;
                }
                continue;
            }

            if (field.key == "level") {
                rec.level = logLevel(field.value.toString().toLower());
            }
        }

        lgs.emplace_back(rec);
    }

    return lgs;
}

using TracePtr = std::shared_ptr<graph::Trace>;

TracePtr makeTrace(const trace::Trace &rawTrace)
{
    auto tracePtr = std::make_shared<graph::Trace>();
    tracePtr->traceID = rawTrace.traceID;
    tracePtr->spans.reserve(rawTrace.spans.size());
    tracePtr->process.reserve(rawTrace.process.size());

    QHash<QString, graph::Process *> processMap;
    processMap.reserve(rawTrace.process.size());

    for (auto processIter = rawTrace.process.begin(); processIter != rawTrace.process.end();
         ++processIter) {
        auto process = std::make_unique<graph::Process>();

        process->name = processIter->name;
        processMap[processIter.key()] = process.get();
        process->tags = copyTags(processIter->tags);
        tracePtr->process.emplace_back(std::move(process));
    }

    QHash<QString, graph::Span *> spanMap;
    spanMap.reserve(rawTrace.spans.size());

    for (auto spanIter = rawTrace.spans.begin(); spanIter != rawTrace.spans.end(); ++spanIter) {
        auto span = std::make_unique<graph::Span>();

        span->spanID = spanIter->spanID;
        span->operationName = spanIter->operationName;
        span->startTime = spanIter->startTime;
        span->duration = spanIter->duration;
        span->process = processMap.value(spanIter->processID, nullptr);
        span->tags = copyTags(spanIter->tags);
        span->logs = copyLogs(spanIter->logs);

        spanMap.insert(span->spanID, span.get());
        tracePtr->spans.emplace_back(std::move(span));
    }

    for (auto spanIter = rawTrace.spans.begin(); spanIter != rawTrace.spans.end(); ++spanIter) {
        auto span = spanMap[spanIter->spanID];
        if (spanIter->references.empty()) {
            tracePtr->root = span;
        }

        for (const auto &ref : spanIter->references) {
            if (ref.refType == trace::SpanReference::Type::ChildOf) {
                auto parent = spanMap.value(ref.spanID, nullptr);
                if (parent == nullptr) {
                    qCritical() << "cant found parent span";
                    continue;
                }

                parent->children.push_back(span);
                span->parent = parent;
            }
        }
    }
    for (auto spanIter = rawTrace.spans.begin(); spanIter != rawTrace.spans.end(); ++spanIter) {
        auto span = spanMap[spanIter->spanID];
        if (span != tracePtr->root) {
            span->shift = span->startTime - tracePtr->root->startTime;
        }
    }

    return tracePtr;
};

} // namespace

namespace graph {

std::shared_ptr<TraceGraph> TraceGraph::makeGraph(const trace::TraceDocument &document)
{
    auto traceGraph = std::make_shared<TraceGraph>();

    traceGraph->traces.reserve(document.traces.size());
    for (const auto &trace : document.traces) {
        traceGraph->traces.emplace_back(makeTrace(trace));
    }
    return traceGraph;
}

} // namespace graph