cmake_minimum_required(VERSION 3.19)
project(parsing)

set(CMAKE_CXX_STANDARD 17)

add_executable(parsing main.cpp data_stream.cpp Parsing.cpp M_XML.cpp)