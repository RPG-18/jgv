#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "trace.h"

namespace {

void setError(trace::TraceParseError *error, trace::TraceParseError::ParseError err)
{
    if (error != nullptr) {
        error->error = err;
    }
}

std::vector<trace::SpanReference> parseSpanReference(const QJsonArray &array,
                                                     trace::TraceParseError *error)
{
    if (array.isEmpty()) {
        return {};
    }

    std::vector<trace::SpanReference> refs;
    refs.reserve(array.size());

    for (auto jRef : array) {
        if (!jRef.isObject()) {
            setError(error, trace::TraceParseError::ParseError::InvalidJSON);
            qWarning() << "invalid trace, expected object in references list";
            return {};
        }

        auto jObj = jRef.toObject();
        if (jObj["refType"].toString() != "CHILD_OF") {
            qCritical() << "invalid trace, unknown refType" << jObj["refType"].toString();
        }
        trace::SpanReference ref;
        ref.refType = trace::SpanReference::Type::ChildOf;
        ref.traceID = jObj["traceID"].toString();
        ref.spanID = jObj["spanID"].toString();
        refs.emplace_back(ref);
    }

    return refs;
}

trace::Tags parseTags(const QJsonArray &array, trace::TraceParseError *error)
{
    if (array.isEmpty()) {
        return {};
    }

    trace::Tags tags;
    tags.reserve(array.size());
    for (auto item : array) {
        auto jObj = item.toObject();

        trace::Tag tag;
        tag.key = jObj["key"].toString();

        const auto tagType = jObj["type"].toString();
        const auto value = jObj["value"];

        if (tagType == "string") {
            tag.value = value.toString();
        } else if (tagType == "int64") {
            tag.value = value.toInteger();
        } else if (tagType == "bool") {
            tag.value = value.toBool();
        } else {
            qCritical() << "Unknown tag type" << tagType;
        }

        tags.emplace_back(tag);
    }
    return tags;
}

QVector<trace::LogRecord> parseLogs(const QJsonArray &array, trace::TraceParseError *error)
{
    if (array.isEmpty()) {
        return {};
    }

    QVector<trace::LogRecord> logs;
    logs.reserve(array.size());

    for (auto item : array) {
        auto jObj = item.toObject();

        trace::LogRecord lr;
        lr.timestamp = trace::TimePoint(std::chrono::microseconds(jObj["timestamp"].toInteger()));

        QVector<trace::LogRecord::Field> fields;
        auto jFields = jObj["fields"].toArray();
        fields.reserve(jFields.size());

        for (auto fieldItem : jFields) {
            auto jFieldObj = fieldItem.toObject();
            trace::LogRecord::Field field;
            field.key = jFieldObj["key"].toString();
            const auto fieldType = jFieldObj["type"].toString();
            const auto value = jFieldObj["value"];

            if (fieldType == "string") {
                field.value = value.toString();
            } else if (fieldType == "int64") {
                field.value = value.toInteger();
            } else if (fieldType == "bool") {
                field.value = value.toBool();
            } else {
                qCritical() << "Unknown log field type " << fieldType;
            }
            fields.emplace_back(field);
        }

        lr.fields.swap(fields);
        logs.emplace_back(lr);
    }

    return logs;
}

trace::Span parseSpan(const QJsonObject &spanObj, trace::TraceParseError *error)
{
    trace::Span span;

    span.traceID = spanObj["traceID"].toString();
    span.spanID = spanObj["spanID"].toString();
    span.flags = spanObj["flags"].toInt();
    span.operationName = spanObj["operationName"].toString();
    span.references = std::move(parseSpanReference(spanObj["references"].toArray(), error));
    span.startTime = trace::TimePoint(std::chrono::microseconds(spanObj["startTime"].toInteger()));
    span.duration = std::chrono::microseconds(spanObj["duration"].toInteger());
    span.tags = std::move(parseTags(spanObj["tags"].toArray(), error));
    span.logs = std::move(parseLogs(spanObj["logs"].toArray(), error));
    span.processID = spanObj["processID"].toString();

    return span;
}

QHash<QString, trace::Process> parseProcess(const QJsonObject &object, trace::TraceParseError *error)
{
    QHash<QString, trace::Process> processes;
    processes.reserve(object.size());

    for (auto iter = object.begin(); iter != object.end(); ++iter) {
        const auto jObj = iter.value().toObject();
        trace::Process process;

        process.name = jObj["serviceName"].toString();
        process.tags = std::move(parseTags(jObj["tags"].toArray(), error));

        processes.insert(iter.key(), std::move(process));
    }

    return processes;
}

trace::Trace parseTrace(const QJsonObject &traceObj, trace::TraceParseError *error) noexcept
{
    trace::Trace tr;
    tr.traceID = traceObj["traceID"].toString();

    if (tr.traceID.isEmpty()) {
        qWarning() << "invalid trace, empty traceID";
        setError(error, trace::TraceParseError::ParseError::InvalidJSON);
        return {};
    }

    auto jSpans = traceObj["spans"].toArray();
    if (jSpans.isEmpty()) {
        qWarning() << "invalid trace, empty spans";
        setError(error, trace::TraceParseError::ParseError::InvalidJSON);
        return {};
    }

    tr.spans.reserve(jSpans.size());

    for (auto jSpan : jSpans) {
        auto span = parseSpan(jSpan.toObject(), error);
        if (span.isEmpty()) {
            return {};
        }

        tr.spans.emplaceBack(span);
    }

    auto jProceess = traceObj["processes"].toObject();
    if (jProceess.isEmpty()) {
        qWarning() << "invalid trace, empty processes";
        setError(error, trace::TraceParseError::ParseError::InvalidJSON);
        return {};
    }

    tr.process = std::move(parseProcess(jProceess, error));
    return tr;
}

} // namespace
namespace trace {

bool Trace::isEmpty() const noexcept
{
    return traceID.isEmpty() || spans.isEmpty();
}

TraceDocument TraceDocument::parseDocument(const QByteArray &data, TraceParseError *error) noexcept
{
    QJsonParseError jErr;
    auto jDoc = QJsonDocument::fromJson(data, &jErr);

    if (jErr.error != QJsonParseError::NoError) {
        setError(error, TraceParseError::ParseError::InvalidJSON);
        qWarning() << "invalid trace json" << jErr.errorString();
        return {};
    }

    if (!jDoc.isObject()) {
        qWarning() << "invalid trace json, expected object";
        setError(error, TraceParseError::ParseError::InvalidJSON);
        return {};
    }

    auto jData = jDoc["data"];
    if (!jData.isArray()) {
        qWarning() << "invalid trace json, data expected array";
        setError(error, TraceParseError::ParseError::InvalidJSON);
        return {};
    }

    auto traces = jData.toArray();
    if (traces.isEmpty()) {
        return {};
    }

    TraceDocument doc;

    doc.traces.reserve(traces.size());

    for (auto jTrace : traces) {
        if (!jTrace.isObject()) {
            qWarning() << "invalid trace json, data expected array of objects";
            setError(error, TraceParseError::ParseError::InvalidJSON);
            return {};
        }

        auto trace = parseTrace(jTrace.toObject(), error);
        doc.traces.emplaceBack(trace);
    }

    return doc;
}

QString TraceParseError::errorString() const
{
    switch (error) {
    case ParseError::NoError:
        return QLatin1String("not error");
    case ParseError::InvalidJSON:
        return QLatin1String("invalid json");
    }

    return QString();
}

} // namespace trace