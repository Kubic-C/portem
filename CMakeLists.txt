cmake_minimum_required(VERSION "3.12")
project(PORTEM)

add_compile_options("-fsanitize=address")
add_link_options("-fsanitize=address")

add_subdirectory("src")
add_subdirectory("test")