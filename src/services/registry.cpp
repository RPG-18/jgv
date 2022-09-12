#include "registry.h"

namespace services {
Registry *Registry::instance() noexcept
{
    static Registry obj;
    return &obj;
}

void Registry::setLogger(const std::shared_ptr<LogService> &logger)
{
    m_logger = logger;
}

LogService *Registry::logger()
{
    return m_logger.get();
}

} // namespace services
