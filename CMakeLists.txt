cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(LLZX_MakingWheels VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 添加公共工具库
add_subdirectory(common)

# 添加子项目
option(BUILD_CACHE_SYSTEM "Build cache system project" ON)
option(BUILD_MEMORY_POOL "Build memory pool project" ON)

if(BUILD_CACHE_SYSTEM)
    add_subdirectory(cache_system)
endif()

if(BUILD_MEMORY_POOL)
    add_subdirectory(memory_pool)
endif()

# 提供构建所有项目的选项
add_custom_target(build_all
    COMMENT "Building all projects"
    DEPENDS
        $<TARGET_NAME_IF_EXISTS:cache_system>
        $<TARGET_NAME_IF_EXISTS:memory_pool>
)

# 提供清理所有构建产物的选项
add_custom_target(clean_all
    COMMENT "Cleaning all build files"
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)