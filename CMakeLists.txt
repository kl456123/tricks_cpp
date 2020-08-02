cmake_minimum_required(VERSION 3.2)

project(tricks_cpp)

find_package(Glog REQUIRED)
include_directories(${CMAKE_SOURCE_DIR})


add_executable(threadpool_test
    multi_threads/main.cc
    multi_threads/simple_threadpool.cc)

target_link_libraries(threadpool_test -pthread glog::glog)
