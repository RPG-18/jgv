#pragma once

#include "process.h"
#include "span.h"

namespace trace {
struct Trace
{
    QString traceID;
    QVector<Span> spans;
    QHash<QString, Process> process;

    bool isEmpty() const noexcept;
};

struct TraceParseError
{
    enum class ParseError {
        NoError,
        InvalidJSON,
    };

    ParseError error = ParseError::NoError;

    QString errorString() const;
};

struct TraceDocument
{
    QVector<Trace> traces;

    static TraceDocument parseDocument(const QByteArray &data,
                                       TraceParseError *error = nullptr) noexcept;
};

} // namespace trace
