#include <algorithm>

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QTextStream>

#include "flat_logs.h"

namespace {
QString levelToString(graph::LogRecord::Level level)
{
    static const QHash<graph::LogRecord::Level, QString> levels = {
        {graph::LogRecord::Level::No, ""},
        {graph::LogRecord::Level::Debug, "D"},
        {graph::LogRecord::Level::Info, "I"},
        {graph::LogRecord::Level::Warn, "W"},
        {graph::LogRecord::Level::Error, "E"},
        {graph::LogRecord::Level::Panic, "P"},
        {graph::LogRecord::Level::Fatal, "F"},
    };

    return levels.value(level, "");
}

QString levelToColor(graph::LogRecord::Level level)
{
    static const QHash<graph::LogRecord::Level, QString> levels = {
        {graph::LogRecord::Level::No, "#FFFFFF"},
        {graph::LogRecord::Level::Debug, "#01DF74"},
        {graph::LogRecord::Level::Info, "#0040FF"},
        {graph::LogRecord::Level::Warn, "#DF7401"},
        {graph::LogRecord::Level::Error, "#DF0101"},
        {graph::LogRecord::Level::Panic, "#FE2E2E"},
        {graph::LogRecord::Level::Fatal, "#FE2E2E"},
    };

    return levels.value(level, "#FFFFFF");
}

} // namespace

namespace components {

FlatLogModel::FlatLogModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_headers << "Level"
              << "Time"
              << "Span"
              << "Process"
              << "";
}

const TraceGraph &FlatLogModel::getGraph()
{
    return m_graph;
}

void FlatLogModel::setGraph(const TraceGraph &data)
{
    m_graph = data;
    if (m_graph.data != nullptr) {
        emit notifyGraphChanged();
        makeIndexes();
    }
}

int FlatLogModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return m_indexes.size();
}

int FlatLogModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return m_headers.size();
}

QVariant FlatLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= m_indexes.size()) {
        return {};
    }

    if (role < Qt::DisplayRole) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        role = Level + index.column();
    }
    auto &logIndex = m_indexes[index.row()];
    auto recordIdx = logIndex.logIndex;
    auto const &logRecord = logIndex.span->logs[recordIdx];

    switch (role) {
    case Level:
        return levelToString(logRecord.level);
    case RawLevel:
        return QVariant::fromValue(logRecord.level);
    case LevelColor:
        return levelToColor(logRecord.level);
    case Time: {
        const int64_t msec = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 logRecord.timestamp.time_since_epoch())
                                 .count();
        const auto dt = QDateTime::fromMSecsSinceEpoch(msec);
        return dt.time().toString("hh:mm:ss.zzz");
    }
    case Span:
        return logIndex.span->spanID;
    case Process:
        return logIndex.span->process->name;
    case Message:
        return QVariant::fromValue(logRecord.fields);
    case HasError:
        return logRecord.hasError;
    }

    return {};
}

QVariant FlatLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }
    if (orientation != Qt::Horizontal) {
        return section;
    }

    return m_headers.at(section);
}

void FlatLogModel::makeIndexes()
{
    int logsRecords = 0;
    for (int i = 0; i < m_graph.data->traces.size(); i++) {
        const auto &trace = m_graph.data->traces[i];
        for (int j = 0; j < trace->spans.size(); j++) {
            logsRecords += trace->spans[j]->logs.size();
        }
    }
    if (logsRecords == 0) {
        return;
    }

    beginResetModel();
    m_indexes.reserve(logsRecords);
    for (int i = 0; i < m_graph.data->traces.size(); i++) {
        const auto &trace = m_graph.data->traces[i];
        for (int j = 0; j < trace->spans.size(); j++) {
            auto span = trace->spans[j].get();

            for (int k = 0; k < span->logs.size(); k++) {
                LogIndex idx = {span, k};
                m_indexes.emplace_back(idx);
            }
        }
    }

    std::sort(m_indexes.begin(), m_indexes.end(), [](const auto &i, const auto &j) -> bool {
        return i.span->logs[i.logIndex].timestamp < j.span->logs[j.logIndex].timestamp;
    });
    endResetModel();
}

QHash<int, QByteArray> FlatLogModel::roleNames() const
{
    static QHash<int, QByteArray> roles{{Qt::DisplayRole, "display"},
                                        {Level, "level"},
                                        {Time, "time"},
                                        {Span, "span"},
                                        {Process, "process"},
                                        {Message, "message"},
                                        {LevelColor, "levelColor"},
                                        {HasError, "hasError"}

    };
    return roles;
}

FieldsModel::FieldsModel(QObject *parent)
    : QAbstractListModel(parent)
{}

void FieldsModel::setFields(const QVariant &data)
{
    beginResetModel();
    m_fields = data.value<QVector<graph::LogRecord::Field>>();
    emit notifyFieldsChanged();
    endResetModel();
}

int FieldsModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return m_fields.size();
}

QHash<int, QByteArray> FieldsModel::roleNames() const
{
    static QHash<int, QByteArray> roles{{Key, "key"}, {Value, "value"}};
    return roles;
}

QVariant FieldsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= m_fields.size()) {
        return {};
    }

    const auto row = index.row();
    switch (role) {
    case Key:
        return m_fields[row].key;
    case Value:
        return m_fields[row].value.toString();
    default:
        return {};
    }
}

LogLevelModel::LogLevelModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_records << Record{false, graph::LogRecord::Level::Debug, "Debug"}
              << Record{false, graph::LogRecord::Level::Info, "Info"}
              << Record{false, graph::LogRecord::Level::Warn, "Warn"}
              << Record{false, graph::LogRecord::Level::Error, "Error"}
              << Record{false, graph::LogRecord::Level::Panic, "Panic"}
              << Record{false, graph::LogRecord::Level::Fatal, "Fatal"};
    m_selected.reserve(m_records.size());
}

int LogLevelModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return m_records.size();
}

QHash<int, QByteArray> LogLevelModel::roleNames() const
{
    static QHash<int, QByteArray> roles{{Level, "level"}, {IsChecked, "isChecked"}};
    return roles;
}

QVariant LogLevelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= m_records.size()) {
        return {};
    }

    const auto row = index.row();

    switch (role) {
    case Level:
        return m_records[row].str;
    case IsChecked:
        return m_records[row].checked;
    default:
        return {};
    }
}

void LogLevelModel::checked(int row, bool check)
{
    if (row >= m_records.size()) {
        return;
    }

    if (m_records[row].checked == check) {
        return;
    }

    m_records[row].checked = check;
    if (m_records[row].checked) {
        m_selected.insert(m_records[row].level);
    } else {
        m_selected.remove(m_records[row].level);
    }

    const auto itemIndex = index(row, 0, QModelIndex());
    changePersistentIndex(itemIndex, itemIndex);

    emit selectedLevels(m_selected);
}

FilteredLogModel::FilteredLogModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_logLevel(nullptr)
{}

bool FilteredLogModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    auto level = index.data(FlatLogModel::RawLevel).value<graph::LogRecord::Level>();
    auto process = index.data(FlatLogModel::Process).toString();

    bool byLevel = true;
    if (!m_selectedLevels.isEmpty()) {
        byLevel = m_selectedLevels.contains(level);
    }

    bool byProcess = true;
    if (!m_selectedProcess.isEmpty()) {
        byProcess = m_selectedProcess.contains(process);
    }

    return byLevel && byProcess;
}

LogLevelModel *FilteredLogModel::logLevel()
{
    return m_logLevel;
}

void FilteredLogModel::setLogLevel(LogLevelModel *level)
{
    m_logLevel = level;
    emit notifyLogLevelChanged();
    QObject::connect(m_logLevel,
                     &LogLevelModel::selectedLevels,
                     this,
                     &FilteredLogModel::onSelectedLevels);
}

ProcessModel *FilteredLogModel::process()
{
    return m_process;
}

void FilteredLogModel::setProcess(ProcessModel *process)
{
    m_process = process;
    emit notifyProcessChanged();
    QObject::connect(m_process,
                     &ProcessModel::selectedProcess,
                     this,
                     &FilteredLogModel::onSelectedProcess);
}

void FilteredLogModel::onSelectedLevels(const QSet<graph::LogRecord::Level> &set)
{
    beginResetModel();
    m_selectedLevels = set;
    endResetModel();
}

void FilteredLogModel::onSelectedProcess(const QSet<QString> &set)
{
    beginResetModel();
    m_selectedProcess = set;
    endResetModel();
}

ProcessModel::ProcessModel(QObject *parent)
    : QAbstractListModel(parent)
{}

const TraceGraph &ProcessModel::getGraph()
{
    return m_graph;
}

void ProcessModel::setGraph(const TraceGraph &data)
{
    m_graph = data;
    if (m_graph.data != nullptr) {
        emit notifyGraphChanged();
        makeRecords();
    }
}

void ProcessModel::makeRecords()
{
    beginResetModel();
    m_records.clear();
    for (int i = 0; i < m_graph.data->traces.size(); i++) {
        const auto &trace = m_graph.data->traces[i];
        for (int j = 0; j < trace->process.size(); ++j) {
            Record rec{false, trace->process[j]->name};
            m_records.emplace_back(rec);
        }
    }

    endResetModel();
}

int ProcessModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return m_records.size();
}

QHash<int, QByteArray> ProcessModel::roleNames() const
{
    static QHash<int, QByteArray> roles{{Process, "processName"}, {IsChecked, "isChecked"}};
    return roles;
}

QVariant ProcessModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= m_records.size()) {
        return {};
    }

    const auto row = index.row();
    switch (role) {
    case Process:
        return m_records[row].name;
    case IsChecked:
        return m_records[row].checked;
    default:
        return {};
    }
}

void ProcessModel::checked(int row, bool check)
{
    if (row >= m_records.size()) {
        return;
    }

    if (m_records[row].checked == check) {
        return;
    }

    m_records[row].checked = check;
    if (m_records[row].checked) {
        m_selectedProcess.insert(m_records[row].name);
    } else {
        m_selectedProcess.remove(m_records[row].name);
    }

    const auto itemIndex = index(row, 0, QModelIndex());
    changePersistentIndex(itemIndex, itemIndex);

    emit selectedProcess(m_selectedProcess);
}

} // namespace components