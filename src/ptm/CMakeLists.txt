

add_library(portem STATIC "./base.hpp")

target_sources(portem PRIVATE
    "./allocator.hpp" "./allocator.cpp"
    "./memory_pool.hpp" "./memory_pool.cpp"
    "./doubly_linked_list.hpp")

target_sources(portem PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/portem.hpp")
