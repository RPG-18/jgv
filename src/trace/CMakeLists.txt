add_library(trace STATIC
        span.h span.cpp
        trace.cpp trace.h
        process.h tag.h
)

target_compile_definitions(trace
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(trace
        PRIVATE
        Qt6::Core
)