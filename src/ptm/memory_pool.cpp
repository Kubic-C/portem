#include "memory_pool.hpp"

namespace ptm {
    _impl_continuous_memory_pool_t::_impl_continuous_memory_pool_t(size_t element_bytesize, size_t max_elements) {
        reset(element_bytesize, max_elements);
    }

    bool _impl_continuous_memory_pool_t::reset(size_t bytesize_of_element, size_t max_elements) {
        const size_t alignment = 16;
        
        cache.last_free = 0;

        this->bytesize_of_element = bytesize_of_element, 
        this->max_elements = max_elements; 
        
        flags_bytesize = max_elements / bits_per_byte;

        // this insures that _elements() returns an aligned address
        flags_bytesize += alignment - (flags_bytesize % alignment);

        elements_bytesize = max_elements * bytesize_of_element;

        if(memory)
            free(memory);
        
        memory = malloc(elements_bytesize + flags_bytesize);
        if(!memory) {
            log("Malloc failed to allocate");
            throw std::exception();
        }

        memset(memory, 0, flags_bytesize);

        return memory != nullptr;
    }

    bool _impl_continuous_memory_pool_t::_attempt_realloc(size_t new_max_elements) {
        uint8_t* flags    = _flags();
        uint8_t* elements = _elements();

        size_t new_flags_bytesize   = new_max_elements + (new_max_elements % bits_per_byte);
        size_t new_elements_bytesize = new_max_elements * bytesize_of_element;

        uint8_t* new_memory = (uint8_t*)realloc(memory, new_flags_bytesize + new_elements_bytesize);
        if(new_memory == nullptr)
            return false;

        assert(new_memory == (uint8_t*)memory);

        // copy over the old flags
        memcpy(new_memory + new_elements_bytesize, _flags(), flags_bytesize);
        // any bytes that weren't covered by the memcpy set to zero
        memset(new_memory + new_elements_bytesize + flags_bytesize, 0, new_flags_bytesize - flags_bytesize);

        flags_bytesize = new_flags_bytesize;
        elements_bytesize = new_elements_bytesize;

        return true;      
    }

    _impl_continuous_memory_pool_t::~_impl_continuous_memory_pool_t() {
        if(memory)
            free(memory);
    }

    _impl_continuous_memory_pool_t::_impl_continuous_memory_pool_t(_impl_continuous_memory_pool_t&& other) {
        cache = other.cache;
        bytesize_of_element = other.bytesize_of_element;
        max_elements        = other.max_elements;
        flags_bytesize      = other.flags_bytesize;
        elements_bytesize   = other.elements_bytesize;
        memory              = other.memory;

        other.memory = nullptr; 
    }

    size_t _impl_continuous_memory_pool_t::try_allocate_in_range(size_t begin, size_t end, size_t n) {
        size_t elements_index = SIZE_MAX;
        
        size_t i = begin;
        for(; i < end;) {
            size_t bit = i;

            for(; bit < max_elements; bit++) {
                if(!is_free(bit)) {
                    elements_index = SIZE_MAX;
                    break;
                } else {
                    if(elements_index == SIZE_MAX) {
                        elements_index = bit;
                    }

                    if((bit - elements_index) + 1 >= n) {
                        goto success;
                    }
                }
            }

            i = bit + 1;
        }

        return SIZE_MAX;
    success:
        return elements_index;
    }

    void* _impl_continuous_memory_pool_t::allocate(size_t n) {
        size_t elements_index = try_allocate_in_range(cache.last_free, max_elements, n);

        if(elements_index == SIZE_MAX) {
            elements_index = try_allocate_in_range(0, cache.last_free, n);

            if(elements_index == SIZE_MAX) {
                return (void*)nullptr;
            }
        } 

        cache.last_free = 0;

        for(size_t i = elements_index; i < elements_index + n; i++) {
            flip_bit(i);
        }

        return (void*)inc_by_byte(_elements(), elements_index * bytesize_of_element);
    }

    void  _impl_continuous_memory_pool_t::deallocate(void* elements, size_t n) {
        size_t elements_index = ((uint8_t*)elements - _elements()) / bytesize_of_element;

        cache.last_free = elements_index;

        for(size_t i = elements_index; i < elements_index + n; i++) {
            assert(!is_free(i));

            flip_bit(i);
        }
    }

    _impl_sparse_memory_pool_t::_impl_sparse_memory_pool_t(_impl_sparse_memory_pool_t&& other) {
        bytesize_of_element = other.bytesize_of_element;
        pools = std::move(other.pools);
    }

    _impl_sparse_memory_pool_t& _impl_sparse_memory_pool_t::operator=(_impl_sparse_memory_pool_t&& other) {
        if(this == &other)
            return *this;
        
        bytesize_of_element = other.bytesize_of_element;
        pools = std::move(other.pools);
        
        return *this;
    }
}