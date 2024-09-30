cmake_minimum_required (VERSION 3.8)

project ("ModernCpp-ConcurrentProgramming-Tutorial")

set(CMAKE_CXX_STANDARD 20)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if(MSVC)
    add_compile_options("/utf-8" "/permissive-" "/Zc:nrvo" "/openmp")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options("-finput-charset=UTF-8" "-fexec-charset=UTF-8" "-fopenmp")
endif()

add_executable(${PROJECT_NAME} "45原子特化shared_ptr.cpp")

set(SFML_DIR "D:/lib/SFML-2.6.1-windows-vc17-64-bit/SFML-2.6.1/lib/cmake/SFML")
find_package(SFML 2.6.1 COMPONENTS system window graphics audio network REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-system sfml-window sfml-graphics sfml-audio sfml-network)

set(fmt_DIR "D:/lib/fmt_x64-windows/share/fmt")
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)

find_package(Qt6 REQUIRED Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

# 当前环境可以直接查找到 vcpkg 的 Boost_DIR 但是却无法查找到 include 路径，手动设置
set(Boost_INCLUDE_DIR "D:/vcpkg-master/installed/x64-windows/include")
include_directories(${Boost_INCLUDE_DIR})
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::system)
