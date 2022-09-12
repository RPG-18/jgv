#pragma once

#include <chrono>
#include <vector>

#include <QtCore/QString>

#include "tag.h"

namespace graph {
using TimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;

struct Process;

struct LogRecord
{
    enum class Level { No, Debug, Info, Warn, Error, Panic, Fatal };

    struct Field
    {
        QString key;
        QVariant value;
    };

    TimePoint timestamp;
    //!< level we take from fields by 'level' field
    Level level = Level::No;
    bool hasError = false;
    QVector<Field> fields;
};

struct Span
{
    Span *parent = nullptr;
    std::vector<Span *> children;
    QString traceID;
    QString spanID;
    QString operationName;
    TimePoint startTime;
    std::chrono::microseconds duration;
    std::chrono::microseconds shift; // startTime - root.startTime
    Process *process = nullptr;
    Tags tags;
    QVector<LogRecord> logs;
};

} // namespace graph