add_library(services STATIC
        registry.cpp registry.h
        log_service.cpp log_service.h
)

target_compile_definitions(services
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(services
        Qt6::Core
)