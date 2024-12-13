# 将指定路径下的所有源文件加入到需要编译的源文件列表
# aux_source_directory(. SRC_LIST)
set(SRC_LIST chatservice.cpp ../../rpc.pb.cc)
aux_source_directory(../../mysql MYSQL_LIST)
aux_source_directory(../../sqlopr SQLOPR_LIST)

# 生成可执行文件server, 以及指定需要编译的源文件列表
add_executable(chat ${SRC_LIST} ${MYSQL_LIST} ${SQLOPR_LIST})

# 表示生成可执行文件需要链接的库文件
target_link_libraries(chat protobuf myrpc muduo_net muduo_base pthread zookeeper_mt mysqlclient)