cmake_minimum_required(VERSION 3.27)
project(ERDOS)

set(CMAKE_CXX_STANDARD 17)

add_executable(ERDOS erdos.cpp bb_covering_tree.cpp cc_embedded_graph.cpp edgecode.cpp find_a_trail.cpp make_cc.cpp ply_to_embedding.cpp)
