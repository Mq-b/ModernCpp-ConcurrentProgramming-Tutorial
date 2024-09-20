cmake_minimum_required (VERSION 3.8)

project ("ModernCpp-ConcurrentProgramming-Tutorial")

set(CMAKE_CXX_STANDARD 17)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if(MSVC)
    add_compile_options("/utf-8" "/permissive-" "/Zc:nrvo")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options("-finput-charset=UTF-8" "-fexec-charset=UTF-8")
endif()

add_executable(${PROJECT_NAME} "25线程安全的队列.cpp")


# 设置 SFML 的 CMake 路径
set(SFML_DIR "D:/lib/SFML-2.6.1-windows-vc17-64-bit/SFML-2.6.1/lib/cmake/SFML")

# 查找 SFML 库并设置链接选项
find_package(SFML 2.6.1 COMPONENTS system window graphics audio network REQUIRED)

# 链接 SFML 库到项目
target_link_libraries(${PROJECT_NAME} sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
