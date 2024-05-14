cmake_minimum_required(VERSION 3.5)

project(async_progress_bar VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC YES)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED Widgets)

set(PROJECT_SOURCES
        main.cpp
        async_progress_bar.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)