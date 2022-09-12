#pragma once

#include <chrono>

#include <QtCore/QDateTime>
#include <QtCore/QVariant>
#include <QtCore/QVector>

#include "tag.h"

namespace trace {

using TimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;

struct SpanReference
{
    enum class Type { ChildOf };

    Type refType;
    QString traceID;
    QString spanID;
};

struct LogRecord
{
    TimePoint timestamp;

    struct Field
    {
        QString key;
        QVariant value;
    };

    QVector<Field> fields;
};

struct Span
{
    QString traceID;
    QString spanID;
    int flags;
    QString operationName;
    std::vector<SpanReference> references;
    TimePoint startTime;
    std::chrono::microseconds duration;
    Tags tags;
    QVector<LogRecord> logs;
    QString processID;

    bool isEmpty() const noexcept;
};

} // namespace trace