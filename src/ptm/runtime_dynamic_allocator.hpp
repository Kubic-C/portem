#pragma once

#include "memory_pool.hpp"

namespace ptm {
    // The coolest allocator of them all!!
    // Allows a data type to be registered at runtime then a 
    // memory pool will be created for that type. If allocate
    // or deallocate is called it will then be passed down to that pool
    class rda_t { 
    public:
        template<typename T>
        bool register_type(size_t initial_max_elements) {
            if(_pool_exists<T>())
                return true;

            pools[std::type_index(typeid(T))].init(sizeof(T), initial_max_elements);

            return true;
        }

        template<typename T>
        T* allocate(size_t n) {
            assert(_pool_exists<T>());
            return (T*)_get_pool<T>()->allocate(n);
        }

        template<typename T>
        void deallocate(T* elements, size_t n) {
            assert(_pool_exists<T>());

            _get_pool<T>()->deallocate((void*)elements, n);
        }

    private:
        template<typename T>
        _impl_sparse_memory_pool_t* _get_pool() {
            auto type_index = std::type_index(typeid(T));

            return &pools[type_index];
        }

        template<typename T>
        bool _pool_exists() {
            return pools.find(std::type_index(typeid(T))) != pools.end();
        }

        std::map<std::type_index, _impl_sparse_memory_pool_t> pools;
    };
}