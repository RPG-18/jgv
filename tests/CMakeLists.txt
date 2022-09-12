find_package(Catch2 CONFIG REQUIRED)

include(CTest)
include(Catch)

add_executable(parser_tests parser.cpp)

target_compile_definitions(parser_tests
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(parser_tests
        PRIVATE
        trace
        Catch2::Catch2
        Catch2::Catch2WithMain
        Qt${QT_VERSION_MAJOR}::Core
)

catch_discover_tests(parser_tests
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/testdata"
        )

add_executable(graph_tests graph.cpp)
target_compile_definitions(graph_tests
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(graph_tests
        PRIVATE
        trace
        graph
        Catch2::Catch2
        Catch2::Catch2WithMain
        Qt${QT_VERSION_MAJOR}::Core
        )


