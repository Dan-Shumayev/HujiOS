cmake_minimum_required(VERSION 3.13)



project(ex5 VERSION 1.0 LANGUAGES C CXX)


add_compile_options(-Wall -Wextra -g -fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(sockets sockets.cpp)
add_executable(container container.cpp)
set_property(TARGET sockets container PROPERTY CXX_STANDARD 11)