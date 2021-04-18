cmake_minimum_required(VERSION 2.8.9)
project (osm)
set(CMAKE_CXX_STANDARD 11)
add_library(osm STATIC osm.cpp osm.h)
add_executable(runner runner.cpp)
target_compile_options(osm PRIVATE -Wall -Wextra -Werror)
target_link_libraries(runner osm)

target_compile_options(osm PUBLIC -g -O2 -DNDEBUG -fverbose-asm --save-temps)

