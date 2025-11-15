cmake_minimum_required(VERSION 3.10)

project(cache_system VERSION 1.0.0 LANGUAGES CXX)

# 添加头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common/include
)

# 收集源文件
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# 创建库
add_library(cache_system ${SOURCES})

# 设置库的版本属性
set_target_properties(cache_system PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# 链接公共库
target_link_libraries(cache_system PRIVATE common_utils)

# 创建测试可执行文件（如果有测试文件）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test")
    file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
    add_executable(cache_system_test ${TEST_SOURCES})
    target_link_libraries(cache_system_test PRIVATE cache_system)
    
    # 添加测试
    enable_testing()
    add_test(NAME cache_system_test COMMAND cache_system_test)
endif()