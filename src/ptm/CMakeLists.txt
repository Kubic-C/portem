

add_library(portem STATIC "./base.hpp")

target_sources(portem PRIVATE
    "./allocator.hpp" "./allocator.cpp"
    "./memory_pool.hpp" "./memory_pool.cpp"
    "./stack_allocator.hpp" "./stack_allocator.cpp"
    "./runtime_dynamic_allocator.hpp" "./runtime_dynamic_allocator.cpp"
    "./free_list.hpp" 
    "./pointer.hpp"
    "./static_list.hpp"
    "./doubly_linked_list.hpp")

target_sources(portem PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/portem.hpp")
