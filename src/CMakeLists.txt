

add_subdirectory(ptm)

target_compile_options(portem PUBLIC "-std=c++17")

target_include_directories(portem PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")