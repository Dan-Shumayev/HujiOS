cmake_minimum_required(VERSION 3.13)

project(ex5 VERSION 1.0 LANGUAGES C CXX)

add_compile_options(-Wall -Wextra -Werror -std=c++11 -g -fsanitize=address)
add_link_options(-fsanitize=address)
add_compile_options(-Wall -Wextra -Werror -std=c++11 -g)

add_executable(sockets src/sockets.cpp src/socket.h src/client.h src/server.h src/command.h src/utilities.h src/prints.h)
add_executable(container src/container.cpp src/utilities.h src/prints.h)
set_property(TARGET sockets container PROPERTY CXX_STANDARD 11)