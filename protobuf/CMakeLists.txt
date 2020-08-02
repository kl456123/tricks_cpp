cmake_minimum_required(VERSION 2.8)


project(protobuf)


find_package(Protobuf REQUIRED)

file(GLOB ProtoFiles ${CMAKE_CURRENT_LIST_DIR}/protos/*.proto)
protobuf_generate_cpp(ProtoSources ProtoHeaders ${ProtoFiles})



add_executable(./test_config_parser.cpp ${ProtoSources} ${Protobuf_LIBRARIES})
