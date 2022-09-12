#pragma once

#include <memory>

#include "log_service.h"

namespace services {

class Registry
{
public:
    static Registry *instance() noexcept;

    void setLogger(const std::shared_ptr<LogService> &logger);

    LogService *logger();

private:
    std::shared_ptr<LogService> m_logger;
};

} // namespace services

#define Services services::Registry::instance()
