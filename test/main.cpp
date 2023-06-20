#include <ptm/portem.hpp>

struct object_t {
    float tensor = rand();
    float mass = rand();
    float restitution = rand();
    float density = rand();
    float static_friction = rand();
    float dynamic_friction = rand();

    object_t* body =(object_t*)rand();
    object_t* prev =(object_t*)rand();
    object_t* next =(object_t*)rand();

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
    for(uint32_t i = 0; i < 10; i++) {
        const double test_size = 100000;
        ptm::object_pool_t<object_t> int_pool(5 * i + 1);

        std::vector<object_t> const_values;
        const_values.reserve(test_size);
        for(uint32_t i = 0; i < test_size; i++) {
            const_values.push_back((object_t){});
        }

        std::vector<object_t*> test_values;
        for(uint32_t i = 0; i < test_size; i++) {
            test_values.push_back(int_pool.create(10, const_values[i]));
        }

        for(uint32_t i = 0; i < test_size; i++) {
            for(uint32_t j = 0; j < 10; j++) {
                if(test_values[i][j] != const_values[i]) {
                    printf("a value was found that was not valid\n");

                    for(auto ptr : test_values)
                        int_pool.destruct(ptr, 1);

                    exit(EXIT_FAILURE);
                }
            }
        }

        for(auto ptr : test_values)
                int_pool.destruct(ptr, 1);

        printf("no failure: %u\n", i);
    }

    return 0;
}