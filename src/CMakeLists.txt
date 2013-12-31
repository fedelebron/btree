set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=c++11")
add_executable(main main.cpp)
add_executable(graphviz graphviz.cpp)
set_target_properties(main graphviz
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
