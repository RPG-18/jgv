#include <QtCore/QFile>

#include <catch2/catch_test_macros.hpp>

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

using namespace trace;

TEST_CASE("parse valid trace", "[trace]")
{
    auto data = readAll("hotroad_rachel.json");

    REQUIRE_FALSE(data.isEmpty());

    TraceParseError error;
    auto doc = TraceDocument::parseDocument(data, &error);

    REQUIRE(error.error == TraceParseError::ParseError::NoError);
    REQUIRE(doc.traces.size() == 1);
    REQUIRE_FALSE(doc.traces[0].isEmpty());

    auto spans = doc.traces[0].spans;
    REQUIRE(spans.size() == 51);

    int rootSpans = 0;

    for (const auto &span : spans) {
        REQUIRE_FALSE(span.isEmpty());
        if (span.references.empty()) {
            rootSpans++;
        }
    }

    REQUIRE(rootSpans == 1);

    REQUIRE(doc.traces[0].process.size() == 6);
    auto processes = doc.traces[0].process;
    for (auto iter = processes.begin(); iter != processes.end(); ++iter) {
        REQUIRE_FALSE(iter.key().isEmpty());
        REQUIRE_FALSE(iter.value().name.isEmpty());
        REQUIRE_FALSE(iter.value().tags.isEmpty());
    }
}