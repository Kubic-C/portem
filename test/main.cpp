#include <ptm/portem.hpp>

struct object_t {
    const char* name = "The name";
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

template<typename comparable_t>
void test_memory_pool(size_t test_size) {
    for(uint32_t i = 0; i < 10; i++) {
        ptm::object_pool_t<comparable_t> int_pool(test_size * 10);

        std::vector<comparable_t> const_values;
        const_values.reserve(test_size);
        for(uint32_t i = 0; i < test_size; i++) {
            const_values.push_back((comparable_t){});
        }

        std::vector<comparable_t*> test_values;
        for(uint32_t i = 0; i < test_size; i++) {
            test_values.push_back(int_pool.create(10, const_values[i]));

            if(rand() % 1) {
                int_pool.destroy(test_values.back(), 10);
                test_values[i] = nullptr;
            }
        }

        for(uint32_t i = 0; i < test_size; i++) {
            if(!test_values[i])
                continue;

            for(uint32_t j = 0; j < 10; j++) {
                if(test_values[i][j] != const_values[i]) {
                    printf("a value was found that was not valid\n");

                    for(auto ptr : test_values)
                        int_pool.destroy(ptr, 10);

                    exit(EXIT_FAILURE);
                }
            }
        }

        for(auto ptr : test_values) {
            if(ptr)
                int_pool.destroy(ptr, 10);
        }
    }
}

template<typename T>
void test_rda(ptm::rda_t& rda, size_t test_size) {
    if(!rda.register_type<T>(test_size)) {
        printf("RDA could not register type");
        exit(EXIT_FAILURE);
    }

    for(uint32_t i = 0; i < 10; i++) {
        std::vector<T> const_values;
        const_values.reserve(test_size);
        for(uint32_t i = 0; i < test_size; i++) {
            const_values.push_back((T)rand());
        }

        std::vector<T*> test_values;
        test_values.reserve(test_size);
        for(uint32_t i = 0; i < test_size; i++) {
            test_values.push_back(rda.allocate<T>(10));

            if(test_values[i] == nullptr) {
                printf("RDA Failed to allocate\n");
                exit(EXIT_FAILURE);
            }

            for(int j = 0; j < 10; j++) {
                test_values[i][j] = const_values[i];
            } 

            if(rand() % 1) {
                rda.deallocate(test_values.back(), 10);
                test_values[i] = nullptr;
                continue;
            }
        }

        for(uint32_t i = 0; i < test_size; i++) {
            if(!test_values[i])
                continue;

            for(uint32_t j = 0; j < 10; j++) {
                if(test_values[i][j] != const_values[i]) {
                    printf("a value was found that was not valid\n");

                    for(auto ptr : test_values)
                        rda.deallocate<T>(ptr, 10);

                    exit(EXIT_FAILURE);
                }
            }
        }

        for(auto ptr : test_values) {
            if(ptr)
                rda.deallocate<T>(ptr, 10);
        }
    }
}


int main() {
    constexpr size_t test_size = 1000;

    printf("using test size %llu\n\n\n", test_size);

    {
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
    }

    printf("# testing memory pool and object pool #\n");
    test_memory_pool<object_t>(test_size);

    printf("success\n\n");


    printf("# testing RDA #\n");
    ptm::rda_t rda;
    
    test_rda<int>(rda, test_size);
    test_rda<long int>(rda, test_size);
    test_rda<long long int>(rda, test_size);
    test_rda<uint32_t>(rda, test_size);
    test_rda<uint8_t>(rda, test_size);

    printf("success\n\n");

    return 0;
}