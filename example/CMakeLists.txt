add_executable(callee callee.cpp example.pb.cc)
target_link_libraries(callee mprpc protobuf)

# add_executable(caller caller.cpp example.pb.cc)
# target_link_libraries(caller mprpc protobuf)