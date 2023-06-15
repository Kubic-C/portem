#include <ptm/portem.hpp>

typedef int example_obj_t;

int main() {
    ptm::memory_pool_t<example_obj_t> pool;

    std::vector<example_obj_t, ptm::indirect_allocator_t<example_obj_t>> v1(ptm::indirect_allocator_t<example_obj_t>{&pool});
    std::vector<example_obj_t, ptm::indirect_allocator_t<example_obj_t>> v2(ptm::indirect_allocator_t<example_obj_t>{&pool});

    v1.emplace_back();
    v2.emplace_back();

    return 0;
}