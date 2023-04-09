set(SRC_LISTS
 rpcprovider.cpp
 rpcheader.pb.cc
 rpcchannel.cpp
 zkutil.cpp
 mprpcinit.cpp
)

add_library(mprpc ${SRC_LISTS})
target_link_libraries(mprpc muduo_net muduo_base zookeeper_mt pthread)