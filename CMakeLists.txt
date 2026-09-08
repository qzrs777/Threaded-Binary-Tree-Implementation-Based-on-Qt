cmake_minimum_required(VERSION 3.16)

project(ThreadedBinaryTreeVisualizer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(threaded_tree_core threadedbinarytree.cpp)
target_include_directories(threaded_tree_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(CTest)
if(BUILD_TESTING)
    add_executable(threaded_tree_tests tests/threadedbinarytree_tests.cpp)
    target_link_libraries(threaded_tree_tests PRIVATE threaded_tree_core)
    add_test(NAME threaded_tree_core_tests COMMAND threaded_tree_tests)
endif()

option(BUILD_GUI "Build the Qt Widgets visualizer" ON)
if(BUILD_GUI)
    find_package(QT NAMES Qt6 Qt5 QUIET COMPONENTS Widgets)
    if(QT_FOUND)
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(PROJECT_SOURCES
            main.cpp mainwindow.cpp mainwindow.h
            treenodeitem.cpp treenodeitem.h treeedgeitem.cpp treeedgeitem.h
        )
        if(QT_VERSION_MAJOR GREATER_EQUAL 6)
            qt_add_executable(untitled1 MANUAL_FINALIZATION ${PROJECT_SOURCES})
        else()
            add_executable(untitled1 ${PROJECT_SOURCES})
        endif()
        target_link_libraries(untitled1 PRIVATE threaded_tree_core Qt${QT_VERSION_MAJOR}::Widgets)
        if(QT_VERSION_MAJOR EQUAL 6)
            qt_finalize_executable(untitled1)
        endif()
    else()
        message(WARNING "Qt Widgets was not found; the GUI target will be skipped")
    endif()
endif()
