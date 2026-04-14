cmake_minimum_required(VERSION 3.25)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
project(fdb-tools VERSION 0.1.0 LANGUAGES CXX DESCRIPTION "LEGO Universe FDB database tools")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(lu_assets
    GIT_REPOSITORY https://github.com/LU-Rebuilt/lu-assets.git
    GIT_TAG main
)
FetchContent_MakeAvailable(lu_assets)

FetchContent_Declare(tool_common
    GIT_REPOSITORY https://github.com/LU-Rebuilt/tool-common.git
    GIT_TAG main
)
FetchContent_MakeAvailable(tool_common)

# CLI converter
add_executable(fdb_converter src/fdb_converter.cpp)
target_link_libraries(fdb_converter PRIVATE lu_assets)

# GUI viewer (optional, requires Qt6)
find_package(Qt6 COMPONENTS Widgets Sql QUIET)
if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)
    add_executable(fdb_viewer
        src/fdb_viewer/main.cpp
        src/fdb_viewer/main_window.cpp src/fdb_viewer/main_window.h
    )
    target_link_libraries(fdb_viewer PRIVATE qt_common lu_assets Qt6::Widgets Qt6::Sql)
    target_include_directories(fdb_viewer PRIVATE src/fdb_viewer)
else()
    message(STATUS "Skipping fdb_viewer (Qt6 not found)")
endif()
