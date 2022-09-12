#pragma once

#include <QtCore/QAbstractTableModel>
#include <QtCore/QSet>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QVector>

#include "trace.h"

namespace components {
class FlatLogModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(TraceGraph graph READ getGraph WRITE setGraph NOTIFY notifyGraphChanged)

public:
    enum Roles {
        Level = Qt::UserRole + 1,
        Time,
        Span,
        Process,
        Message,
        LevelColor,
        HasError,
        RawLevel
    };

    explicit FlatLogModel(QObject *parent = nullptr);

    const TraceGraph &getGraph();
    void setGraph(const TraceGraph &data);

    int rowCount(const QModelIndex & = QModelIndex()) const override final;
    int columnCount(const QModelIndex & = QModelIndex()) const override final;
    QVariant data(const QModelIndex &index, int role) const override final;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override final;
    QHash<int, QByteArray> roleNames() const override;

signals:

    void notifyGraphChanged();

private:
    struct LogIndex
    {
        graph::Span *span;
        int logIndex;
    };

    void makeIndexes();

private:
    TraceGraph m_graph;
    QVector<QString> m_headers;
    QVector<LogIndex> m_indexes;
};

class FieldsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QVariant fields WRITE setFields NOTIFY notifyFieldsChanged)

public:
    enum Roles { Key = Qt::UserRole + 1, Value };

    explicit FieldsModel(QObject *parent = nullptr);

    Q_INVOKABLE void setFields(const QVariant &data);

    int rowCount(const QModelIndex & = QModelIndex()) const override final;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override final;

signals:

    void notifyFieldsChanged();

private:
    QVector<graph::LogRecord::Field> m_fields;
};

class LogLevelModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles { Level = Qt::UserRole + 1, IsChecked };

    explicit LogLevelModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const override final;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override final;

    Q_INVOKABLE void checked(int row, bool check);

signals:

    void selectedLevels(const QSet<graph::LogRecord::Level> &set);

private:
    struct Record
    {
        bool checked = false;
        graph::LogRecord::Level level = graph::LogRecord::Level::No;
        QString str;
    };

private:
    QVector<Record> m_records;
    QSet<graph::LogRecord::Level> m_selected;
};

class ProcessModel;

class FilteredLogModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(LogLevelModel *logLevel READ logLevel WRITE setLogLevel NOTIFY notifyLogLevelChanged)
    Q_PROPERTY(ProcessModel *process READ process WRITE setProcess NOTIFY notifyProcessChanged)
public:
    explicit FilteredLogModel(QObject *parent = nullptr);

    LogLevelModel *logLevel();
    void setLogLevel(LogLevelModel *level);

    ProcessModel *process();
    void setProcess(ProcessModel *process);

signals:

    void notifyLogLevelChanged();
    void notifyProcessChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override final;

private slots:

    void onSelectedLevels(const QSet<graph::LogRecord::Level> &set);
    void onSelectedProcess(const QSet<QString> &set);

private:
    LogLevelModel *m_logLevel;
    ProcessModel *m_process;
    QSet<graph::LogRecord::Level> m_selectedLevels;
    QSet<QString> m_selectedProcess;
};

class ProcessModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(TraceGraph graph READ getGraph WRITE setGraph NOTIFY notifyGraphChanged)

public:
    enum Roles { Process = Qt::UserRole + 1, IsChecked };

    explicit ProcessModel(QObject *parent = nullptr);

    const TraceGraph &getGraph();
    void setGraph(const TraceGraph &data);

    int rowCount(const QModelIndex & = QModelIndex()) const override final;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override final;

    Q_INVOKABLE void checked(int row, bool check);

signals:

    void notifyGraphChanged();
    void selectedProcess(const QSet<QString> &set);

private:
    struct Record
    {
        bool checked = false;
        QString name;
    };

    void makeRecords();

private:
    TraceGraph m_graph;
    QVector<Record> m_records;
    QSet<QString> m_selectedProcess;
};
} // namespace components
