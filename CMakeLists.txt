# 设置cmake的最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(myrpc)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 设置可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置项目编译头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/myrpc)
include_directories(${PROJECT_SOURCE_DIR}/include/mysql)
include_directories(${PROJECT_SOURCE_DIR}/include/redis)
include_directories(${PROJECT_SOURCE_DIR}/include/sqlopr)

# 设置项目编译库文件的搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 加载子目录
add_subdirectory(src)