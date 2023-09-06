#pragma once

#include "base.hpp"
#include "allocator.hpp"
#include "doubly_linked_list.hpp"

namespace ptm {
    template<typename T>
    T* inc_by_byte(T* ptr, size_t byte) {
        return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(ptr) + byte);
    }

    // underlying implementation of a memory pool that is not reliant on
    // templates. Memory here is continuous, better for the cache but cannot resize
    class _impl_continuous_memory_pool_t {
    public:
        _impl_continuous_memory_pool_t() {}
        _impl_continuous_memory_pool_t(size_t element_bytesize, size_t max_elements);
        _impl_continuous_memory_pool_t(_impl_continuous_memory_pool_t&& other);
        ~_impl_continuous_memory_pool_t();

        // Allocates a new block deallocating the old one
        // All elements MUST be deallocated. Returns true if reset was successful
        bool reset(size_t element_bytesize, size_t max_elements);

        bool valid() { return (uint8_t*)memory; }
        void* allocate(size_t n);
        void deallocate(void* elements, size_t n);

        void* get_block() { return (void*)_elements(); } 
        bool elements_in_pool(void* ptr) { return _elements() <= (uint8_t*)ptr && (uint8_t*)ptr <= inc_by_byte(_elements(), elements_bytesize); }

        size_t get_max_elements() { return max_elements; }

    private:
        static constexpr size_t bits_per_byte = 8;

        // should not use 
        bool _attempt_realloc(size_t new_max_elements);
        uint8_t* _flags() { return (uint8_t*)memory; }
        uint8_t* _elements() { return inc_by_byte((uint8_t*)memory, flags_bytesize); }

        bool is_free(size_t index) { return !(_flags()[index / bits_per_byte] & (1 << (index % bits_per_byte))); }
        bool flip_bit(size_t index) { return _flags()[index / bits_per_byte] ^= (1 << (index % bits_per_byte)); }        

    private:
        size_t bytesize_of_element = 0;
        size_t max_elements   = 0;
        size_t flags_bytesize = 0;
        size_t elements_bytesize = 0;
        void*  memory         = nullptr;
    };

    class _impl_sparse_memory_pool_t {
    public:
        _impl_sparse_memory_pool_t() {
            bytesize_of_element = SIZE_MAX;
        }

        _impl_sparse_memory_pool_t(_impl_sparse_memory_pool_t&& other);
        _impl_sparse_memory_pool_t& operator=(_impl_sparse_memory_pool_t&& other);

        _impl_sparse_memory_pool_t(size_t bytesize_of_element, size_t initial_max_elements = 100) {
            this->bytesize_of_element = bytesize_of_element;
            pools.emplace_back(bytesize_of_element, initial_max_elements);
        }

        void* allocate(size_t n, const void* hint = 0) {
           void* elements = nullptr;
            
            for(auto& pool : pools) {
                elements = pool.allocate(n);

                if(elements) {
                    return elements;
                }
            }

            size_t prev_max_elements = pools.back().get_max_elements();
            pools.reserve(1);
            pools.emplace_back(bytesize_of_element, prev_max_elements * 2);
            elements = pools.back().allocate(n);

            return elements;
        }

        void deallocate(void* ptr, size_t n) {
            for(auto& pool : pools) {
                if(pool.elements_in_pool(ptr))
                    pool.deallocate(ptr, n);
            }
        }

    private:    
        size_t bytesize_of_element = 0;
        std::vector<_impl_continuous_memory_pool_t> pools;
    };

    template<typename T>
    class memory_pool_t : public allocator_t<T> {
    public:
        memory_pool_t(size_t initial_max_elements = 100) {
            pool = _impl_sparse_memory_pool_t(sizeof(T), initial_max_elements);
        }

        T* allocate(size_t n, const void* hint = 0) override {
            return (T*)pool.allocate(n, hint);
        }

        void deallocate(T* ptr, size_t n) override {
            pool.deallocate((T*)ptr, n);
        }

        memory_pool_t(const memory_pool_t<T>& other) {
            this->initial_size = other.initial_size;
            this->allocators = other.allocators;
        }

        memory_pool_t<T>* operator=(const memory_pool_t<T>& other) {
            this->initial_size = other.initial_size;
            this->allocators = other.allocators;    

            return this;
        }

        ~memory_pool_t() {
            for(auto& allocator : allocators) {
                allocator.free();
            }
        }

        T* allocate(size_t size, const void* hint = 0) override {
            T* ptr = nullptr;

            for(size_t i = allocators.size() - 1; i != SIZE_MAX; i--) {
                ptr = allocators[i].allocate(size);
                if(ptr != nullptr) {
                    return ptr;
                }
            }

            initial_size += size;
            allocators.emplace_back();
            allocators.back().init(initial_size);

            return allocators.back().allocate(size);
        }   

        void deallocate(T* ptr, size_t n = 0) override {
            for(auto& allocator : allocators) {
                if(allocator.ptr_in_bounds(ptr)) {
                    allocator.deallocate(ptr);
                    return;
                }
            }

            assert(!ptr);
        }

        size_t get_initial_size() const {
            return initial_size;
        }

        const std::vector<block_allocator_t<T>>& get_allocators() const {
            return allocators;
        }

    protected:
        size_t initial_size;
        std::vector<block_allocator_t<T>> allocators;
    };

    template<typename T>
    class object_pool_t {
    public:
        object_pool_t(size_t max_size = 100)
            : pool(max_size) {}

        template<typename ... params>
        T* create(size_t size, params&& ... args) {
            T* elements = pool.allocate(size);
        
            for(size_t i = 0; i < size && elements; i++) {
                pool.construct(&elements[i], args...);
            }

            return elements;
        }

        void destroy(T* ptr, size_t size) {
            for(size_t i = 0; i < size; i++) {
                pool.destroy(ptr + i);
            }

            pool.deallocate(ptr, size);
        }
    
    private:
        memory_pool_t<T> pool;
    };

    template<typename T>
    bool operator==(const memory_pool_t<T>& _1, const memory_pool_t<T>& _2) {
        return _1.get_allocators() == _2.get_allocators();
    }

    template<typename T>
    bool operator!=(const memory_pool_t<T>& _1, const memory_pool_t<T>& _2) {
        return _1.get_allocators() != _2.get_allocators();
    }
}