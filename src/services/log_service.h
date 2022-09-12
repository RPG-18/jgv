#pragma once

#include <QtCore/QObject>

namespace services {

class LogService : public QObject
{
    Q_OBJECT

public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Critical,
        Fatal,
    };

    LogService();

    void addMessage(Level leve, const QString &msg);

signals:

    void logChanged();

private:
    struct Record
    {
        Level level;
        QString message;
    };

private:
    QVector<Record> m_records;
};

} // namespace services