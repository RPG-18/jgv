#include <QtQml/QQmlEngine>

#include "flat_logs.h"
#include "helpers.h"
#include "service_map.h"
#include "trace_downloader.h"

namespace components {
void registerTypes()
{
    qmlRegisterType<TraceDownloader>("jaeger", 1, 0, "TraceDownloader");
    qmlRegisterType<FlatLogModel>("jaeger", 1, 0, "FlatLogModel");
    qmlRegisterType<FieldsModel>("jaeger", 1, 0, "FieldsModel");
    qmlRegisterType<LogLevelModel>("jaeger", 1, 0, "LogLevelModel");
    qmlRegisterType<FilteredLogModel>("jaeger", 1, 0, "FilteredLogModel");
    qmlRegisterType<ProcessModel>("jaeger", 1, 0, "ProcessModel");
    qmlRegisterType<ServiceMap>("jaeger", 1, 0, "ServiceMap");
    qmlRegisterType<ServiceMapNodeItem>("jaeger", 1, 0, "ServiceMapNodeItem");
}
} // namespace components