cmake_minimum_required (VERSION 2.6)
project (external_heap)

link_directories(${CMAKE_SOURCE_DIR})

add_executable(test_external_storage test_external_storage.cpp external_storage.h)
target_link_libraries(test_external_storage gtest)

add_executable(test_external_heap test_external_heap.cpp external_heap.h external_storage.h)
