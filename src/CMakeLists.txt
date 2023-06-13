
add_library(portem STATIC
    "base.hpp"
    "allocator.hpp" "allocator.cpp"
    "memory_pool.hpp" "memory_pool.cpp"
    "portem.hpp")

target_include_directories(portem PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")