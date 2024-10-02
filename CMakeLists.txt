# 钱虎cpp©2024版权所有
# 指定最低cmake版本
cmake_minimum_required(VERSION 2.8...3.10)

# 项目名称 大市操
project(dashicao)

# 完美转发那里会有warnings
# 启用C++17
add_compile_options(-Wno-format-security -std=c++17)

# 目标程序
add_executable(dashicao dashicao.cpp read_config.cpp engine.cpp utils.cpp)

# 添加yaml库
include(FetchContent)
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  #GIT_TAG <tag_name> # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_MakeAvailable(yaml-cpp)
target_link_libraries(dashicao PUBLIC yaml-cpp::yaml-cpp) # The library or executable that require yaml-cpp library

# 添加LoggerCpp库
set(LOGGERCPP_RUN_CPPLINT OFF) # 不要运行CPPLINT
set(LOGGERCPP_RUN_CPPCHECK OFF) # 不要运行cppcheck
set(LOGGERCPP_RUN_DOXYGEN OFF) # 不要构建文档
set(LOGGERCPP_BUILD_EXAMPLE OFF) # 不要构建示例程序

FetchContent_Declare(
  LoggerCpp
  GIT_REPOSITORY https://github.com/SRombauts/LoggerCpp.git
  #GIT_TAG <tag_name> # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_MakeAvailable(LoggerCpp)
target_link_libraries(dashicao PUBLIC LoggerCpp) # The library or executable that require LoggerCpp library

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/LoggerCpp/include)