#include <QtCore/QFile>

#include <catch2/catch_test_macros.hpp>

#include "graph/trace.h"
#include "trace/trace.h"

namespace {

QByteArray readAll(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "file not open" << filename << file.errorString();
        return {};
    }

    auto data = file.readAll();
    file.close();

    return data;
}

} // namespace

TEST_CASE("make trace graph", "[graph]")
{
    auto data = readAll("hotroad_rachel.json");

    REQUIRE_FALSE(data.isEmpty());
    trace::TraceParseError error;
    auto doc = trace::TraceDocument::parseDocument(data, &error);

    REQUIRE(error.error == trace::TraceParseError::ParseError::NoError);
    REQUIRE(doc.traces.size() == 1);

    auto traceGraph = graph::TraceGraph::makeGraph(doc);
    REQUIRE(traceGraph != nullptr);
    REQUIRE(traceGraph->traces.size() == 1);

    auto trace = traceGraph->traces.front();
    REQUIRE(trace->root != nullptr);
    REQUIRE_FALSE(trace->root->children.empty());
    REQUIRE(trace->spans.size() == 51);

    int hasNotChildren = 0;
    int hasTimeShift = 0;
    int logWithDefaultLevel = 0;
    for (int i = 0; i < trace->spans.size(); ++i) {
        auto span = trace->spans[i].get();
        if (span != trace->root) {
            REQUIRE(span->parent != nullptr);
        }

        if (span->children.empty()) {
            ++hasNotChildren;
        }

        if (span->shift.count() > 0) {
            ++hasTimeShift;
        }
        for (const auto &rec : span->logs) {
            if (rec.level == graph::LogRecord::Level::No) {
                logWithDefaultLevel++;
            }
        }
    }

    REQUIRE(hasNotChildren == 25);
    REQUIRE(hasTimeShift == 50);
    REQUIRE(logWithDefaultLevel == 0);
}