cmake_minimum_required(VERSION 3.10)

project(cache_system LANGUAGES CXX)

# 设置调试模式和调试标志
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# 添加头文件目录（仅保留本组件的include）
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 检查是否有main函数，如果有则创建可执行文件，否则创建库
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/cache.cpp" CACHE_CPP_CONTENT)
string(FIND "${CACHE_CPP_CONTENT}" "int main(" HAS_MAIN)

if(HAS_MAIN GREATER -1)
    # 创建可执行文件
    add_executable(cache_system ${CMAKE_CURRENT_SOURCE_DIR}/src/cache.cpp)
    message(STATUS "Creating executable for cache_system")
else()
    # 收集源文件
    file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
    
    # 创建库
    add_library(cache_system ${SOURCES})
endif()