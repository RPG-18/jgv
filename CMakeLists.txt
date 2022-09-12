cmake_minimum_required(VERSION 3.21)
project(jgv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
option(JGV_BUILD_TESTS "Build JGV tests" OFF)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(Graphviz REQUIRED)
find_package(Qt6 COMPONENTS Core Qml Quick LinguistTools QuickControls2 Network REQUIRED)

include_directories(src
        ${GRAPHVIZ_INCLUDE_DIR}
)

add_subdirectory(src)


if(JGV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

set(TS_FILES src/jgv_ru_RU.ts)

set(PROJECT_SOURCES
        src/main.cpp
        src/qml.qrc
        src/qtquickcontrols2.conf
        ${TS_FILES}
)

qt_add_executable(jgv
        WIN32
        ${PROJECT_SOURCES}
)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_compile_definitions(jgv
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(jgv
        PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Network
        trace
        graph
        services
        components
)
