cmake_minimum_required (VERSION 3.8)

project ("ModernCpp-ConcurrentProgramming-Tutorial")

set(CMAKE_CXX_STANDARD 17)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if(MSVC)
    add_compile_options("/utf-8" "/permissive-")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options("-finput-charset=UTF-8" "-fexec-charset=UTF-8")
endif()

add_executable(ModernCpp-ConcurrentProgramming-Tutorial "01-HelloWorld.cpp")
