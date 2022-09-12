add_library(components STATIC
        trace.h
        trace_downloader.cpp trace_downloader.h
        helpers.cpp helpers.h
        flat_logs.cpp flat_logs.h
        service_map.cpp service_map.h
        span_model.cpp span_model.h
        tag_model.cpp tag_model.h
)

target_compile_definitions(components
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(components
        PRIVATE
        trace
        graph
        Qt6::Core
        Qt6::Quick
        Qt6::Network
        Qt6::Gui
        ${GRAPHVIZ_GVC_LIBRARY}
        ${GRAPHVIZ_CGRAPH_LIBRARY}
        ${GRAPHVIZ_CDT_LIBRARY}
)