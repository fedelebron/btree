set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=c++11")
add_executable(btree_test btree_test.cpp)
target_link_libraries(btree_test gtest)
add_test(BTreeTest btree_test)
