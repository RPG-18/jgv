#include "log_service.h"

namespace services {

LogService::LogService()
{
    m_records.reserve(100);
}

void LogService::addMessage(LogService::Level level, const QString &msg)
{
    m_records.push_back(Record{level, msg});

    emit logChanged();
}

} // namespace services