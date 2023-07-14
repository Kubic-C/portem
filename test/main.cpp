#include <ptm/portem.hpp>

struct object_t {
    float tensor = rand();
    float mass = rand();
    float restitution = rand();
    float density = rand();
    float static_friction = rand();
    float dynamic_friction = rand();

    object_t* body =(object_t*)(size_t)rand();
    object_t* prev =(object_t*)(size_t)rand();
    object_t* next =(object_t*)(size_t)rand();

    bool operator!=(const object_t& other) {
        return tensor != other.tensor &&    
               mass != other.mass &&
               restitution != other.restitution &&
               density != other.density &&
               static_friction != other.static_friction &&
               dynamic_friction != other.dynamic_friction &&
               body != other.body &&
               prev != other.prev &&
               next != other.next;
    }
};

int main() {
    constexpr size_t test_size = 10000;

    printf("using test size %llu\n\n\n", test_size);

    printf("# testing free_list_t #\n");
    ptm::free_list_t<int>  free_list;
    std::vector<int> id_list;
    std::vector<int> expected_values;

    for(size_t i = 0; i < test_size; i++) {
        id_list.push_back(free_list.insert(i));
        expected_values.push_back(i);
    }

    for(size_t i = 0; i < test_size; i++) {
        if(free_list[id_list[i]] != expected_values[i]) {
            printf("free_list_t test failed\n");
            exit(EXIT_FAILURE);
        }
    }

    for(size_t i = 0; i < test_size; i++) {
        free_list.erase(id_list[i]);
    }

    id_list.clear();
    expected_values.clear();

    printf("success\n\n");

    printf("# testing small_list_t #\n");
    ptm::small_list_t<int, 128> small_list;

    for(size_t i = 0; i < test_size; i++) {
        small_list.emplace_back(i);
        expected_values.push_back(i);
    }

    for(size_t i = 0; i < test_size; i++) {
        if(small_list[i] != expected_values[i]) {
            printf("small_list_t test failed\n");
            exit(EXIT_FAILURE);
        }
    }

    for(size_t i = 0; i < test_size; i++) {
        small_list.pop_back();
    }

    printf("success\n\n");

    printf("# testing memory pool and object pool #\n");
    for(uint32_t i = 0; i < 10; i++) {
        ptm::object_pool_t<object_t> int_pool(5 * i + 1);
        std::bitset<test_size> object_deleted = {};

        std::vector<object_t> const_values;
        const_values.reserve(test_size);
        for(uint32_t i = 0; i < test_size; i++) {
            const_values.push_back((object_t){});
        }

        std::vector<object_t*> test_values;
        for(uint32_t i = 0; i < test_size; i++) {
            test_values.push_back(int_pool.create(10, const_values[i]));

            if(rand() % 1) {
                int_pool.destroy(test_values.back(), 10);
                object_deleted[i] = 1;
            }
        }

        for(uint32_t i = 0; i < test_size; i++) {
            if(object_deleted[i])
                continue;

            for(uint32_t j = 0; j < 10; j++) {
                if(test_values[i][j] != const_values[i]) {
                    printf("a value was found that was not valid\n");

                    for(auto ptr : test_values)
                        int_pool.destroy(ptr, 1);

                    exit(EXIT_FAILURE);
                }
            }
        }

        for(auto ptr : test_values)
                int_pool.destroy(ptr, 1);
    }

    printf("success\n\n");

    return 0;
}