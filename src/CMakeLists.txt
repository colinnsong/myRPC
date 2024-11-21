# 将指定路径下的所有源文件加入到需要编译的源文件列表
aux_source_directory(. SRC_LIST)

# 生成框架的动态库文件
add_library(myrpc ${SRC_LIST})

# 表示生成动态库文件需要链接的库文件
target_link_libraries(myrpc muduo_net muduo_base pthread)