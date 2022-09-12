add_library(graph STATIC
        tag.h
        span.h process.h
        trace.h trace.cpp
)

target_compile_definitions(graph
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(graph
        PRIVATE
        Qt6::Core
)