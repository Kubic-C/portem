#pragma once

#include "base.hpp"
#include "allocator.hpp"
#include "doubly_linked_list.hpp"

namespace ptm {
    template<typename T>
    T* inc_by_byte(T* ptr, size_t byte) {
        return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(ptr) + byte);
    }
    
    enum block_flags_t: uint8_t {
        BLOCK_FLAGS_FREE        = 0,
        BLOCK_FLAGS_INITIALIZED = 1, // not used
        BLOCK_FLAGS_IN_USE      = 1 << 0
    };

    template<typename T>
    struct block_header_t : public doubly_linked_list_element_t {
        block_header_t(size_t total_bytes, std::bitset<8> flags)
            : total_bytes(total_bytes), flags(flags) {}

        ~block_header_t() override {}

        std::bitset<8> flags;

        // the amount of bytes, including the header and its blocks it occupies
        size_t total_bytes; 

        size_t block_count_max() const {
            return (total_bytes - sizeof(block_header_t<T>)) / sizeof(T);
        }

        T* elements() {
            return inc_by_byte((T*)this, sizeof(*this));
        }
    };

    template<typename T>
    class block_allocator_t {
        static constexpr size_t block_header_size = sizeof(block_header_t<T>);
        static constexpr size_t alignment   = system_alignment; 

    public:
        block_allocator_t() {}
        ~block_allocator_t() {}

        void init(size_t capacity = 10) {
            size_t alignment_offset = 0;

            this->capacity_ = (capacity * sizeof(T) + (block_header_size * capacity) / 10);

            this->real_ptr = (uint8_t*)malloc(this->capacity_ + alignment);
            alignment_offset = alignment - ((size_t)real_ptr % alignment);

            if(alignment_offset == alignment) {
                this->aligned_ptr = real_ptr;
            } else {
                this->aligned_ptr = real_ptr + alignment_offset;
            }

            if(!create_block_header(this->aligned_ptr, this->capacity_)) {
                log("could not create allocator");
            }
        }

        block_allocator_t(const block_allocator_t& other) {
            this->aligned_ptr  = other.aligned_ptr;
            this->real_ptr     = other.real_ptr;
            this->capacity_    = other.capacity_;
            this->free_blocks  = other.free_blocks;
            this->total_blocks = other.total_blocks;
            this->freed_blocks = other.freed_blocks;
        }

        void free() {
            assert(real_ptr);            
            
            ::free(real_ptr);
            real_ptr = nullptr;

            if(freed_blocks != total_blocks)
                log("some blocks were not free'd");
        }

        // size is the amount of elements you want allocated
        T* allocate(size_t size) {
            size_t             byte_size = aligned_space_needed(size * sizeof(T));
            block_header_t<T>* header    = get_block_with(byte_size);

            if(header == nullptr) {
                return nullptr;
            }

 
            block_try_bisect(header, byte_size);
            set_block_as_allocated(header);

            return header->elements();
        }

        void deallocate(T* block) {
            block_header_t<T>* header = (block_header_t<T>*)((uint8_t*)block - block_header_size);

            assert(block != nullptr);
            assert(!header->flags[BLOCK_FLAGS_FREE]);

            header->flags[BLOCK_FLAGS_FREE].flip();

            freed_blocks++;

            free_blocks.push_back(header);
        }

        size_t capacity() const {
            return capacity_;
        }

        bool ptr_in_bounds(T* ptr) const {
            return aligned_ptr <= (uint8_t*)ptr && (uint8_t*)ptr <= (aligned_ptr + capacity_);
        }

        uint8_t* get_real_ptr() const {
            return real_ptr;
        }

    protected:
        block_header_t<T>* get_block_with(size_t size) {
            block_header_t<T>* header = nullptr;
            
            header = find_header_with_at_least(size);
            if(header != nullptr) {
                return header;
            }

            header = try_merge_header_with_at_least(size);
            if(header != nullptr) {
                return header;
            }

            return nullptr;
        }

        block_header_t<T>* find_header_with_at_least(size_t size) {
            block_header_t<T>* cur = dynamic_cast<block_header_t<T>*>(free_blocks.first);
            while(cur != nullptr) {
                if(cur->total_bytes >= size) {
                    return cur;
                }

                cur = dynamic_cast<block_header_t<T>*>(cur->next);
            }

            return nullptr;
        }

        block_header_t<T>* try_merge_header_with_at_least(size_t size) {
            // we take advantage of the fact that block headers
            // are right next to each other in memory (with some elements in between)
            // -knowing this allows us to merge multiple headers if they are:
            // 1. free'd and
            // 2. contigous in memory

            size_t             total_contigous_bytes = 0;
            block_header_t<T>* block_begin            = nullptr;
            block_header_t<T>* block_end              = nullptr;

            // there is always a valid header at aligned ptr
            block_header_t<T>*       header = (block_header_t<T>*)aligned_ptr;
            const block_header_t<T>* end    = (block_header_t<T>*)(aligned_ptr + this->capacity_);
            for(; header != end; header = inc_by_byte(header, header->total_bytes)) {
                size_t usable_user_memory = 0;

                if(header->flags[BLOCK_FLAGS_FREE]) {
                    total_contigous_bytes += header->total_bytes;

                    if(block_begin == nullptr) {
                        block_begin = header;
                    }
                } else {
                    total_contigous_bytes = 0;
                    block_begin = nullptr;
                    continue;
                }

                usable_user_memory = total_contigous_bytes - block_header_size;
                if(usable_user_memory >= size - block_header_size) {
                    block_end = (block_header_t<T>*)((uint8_t*)header + header->total_bytes);
                    break;
                }
            }

            if(block_end == nullptr)
                return nullptr;

            // remove all headers from list
            for(auto cur = block_begin; cur != block_end; cur = inc_by_byte(cur, cur->total_bytes)) {
                free_blocks.remove_element(cur);
                total_blocks--;
                freed_blocks--;
            }

            return create_block_header((uint8_t*)block_begin, total_contigous_bytes);
        }

        /**
         * @brief attempts to bisects a free block into two differently sized blocks
         * 
         * @param header the header of the block to bisect
         * @param size the number of bytes to preserve in the block (must include header size)
         * @return the front block
         */
        block_header_t<T>* block_try_bisect(block_header_t<T>* header, size_t size) {
            assert(header->flags[BLOCK_FLAGS_FREE]);
            assert(header->total_bytes >= size);

            int32_t            left_over_block_total_bytes = 0;
            block_header_t<T>* next       = nullptr;

            left_over_block_total_bytes = (int32_t)header->total_bytes - (int32_t)size;
            if(left_over_block_total_bytes > block_header_size) {
                next = inc_by_byte(header, size);
                create_block_header((uint8_t*)next, left_over_block_total_bytes);
            }

            if(next != nullptr)
                header->total_bytes = size;

            return next;
        }

        void set_block_as_allocated(block_header_t<T>* header) {
            assert(header->flags[BLOCK_FLAGS_FREE]);

            // claim the block as allocated
            header->flags[BLOCK_FLAGS_FREE].flip();

            free_blocks.remove_element(header);

            freed_blocks--;
        }

        // size is in bytes
        block_header_t<T>* create_block_header(uint8_t* addr, size_t size) {
            const size_t test_align = ((size_t)addr % alignment);
            assert(((size_t)addr % alignment) == 0);
            assert((int32_t)size - block_header_size >= 0);

            if(addr + size > (aligned_ptr + capacity_) || size < block_header_size) {
                return nullptr;
            }

            block_header_t<T>* header = (block_header_t<T>*)addr;
            // since header will be used as a polymorphic object
            // we must initialize it
            new(header)block_header_t<T>(size, 0);

            header->flags[BLOCK_FLAGS_FREE].flip();

            free_blocks.push_back(header);

            total_blocks++;
            freed_blocks++;

            return header;
        }

        size_t aligned_space_needed(size_t byte_size) {
            size_t total_space      = block_header_size + byte_size;
            size_t alignment_offset = total_space % alignment; 

            return total_space + (alignment - alignment_offset);
        }
 
    private:
        size_t capacity_ = 0; // in bytes

        size_t   total_blocks = 0;
        size_t   freed_blocks = 0;
        uint8_t* real_ptr     = nullptr;
        uint8_t* aligned_ptr  = nullptr;

        doubly_linked_list_header_t<block_header_t<T>> free_blocks;
    };

    template<typename T>
    class memory_pool_t : public allocator_t<T>{
    public:
        template <typename U> 
        struct rebind { typedef memory_pool_t<U> other; };
    public:
        static constexpr size_t _initial_size = 100;

        template<typename U>
        memory_pool_t(const memory_pool_t<U>& other) {
            this->initial_size = other.get_initial_size();
        }

        memory_pool_t() {
            this->initial_size = _initial_size;
        }

        memory_pool_t(size_t initial_size) {
            this->initial_size = initial_size;
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

    /* destroys and creates objects upon creating and destroying */
    template<typename T>
    class object_pool_t {
    public:
        object_pool_t() {}

        object_pool_t(size_t initial_size)
            : mem_pool(initial_size) {}

        template<typename ... params>
        T* create(size_t size, params&& ... args) {
            T* elements = mem_pool.allocate(size);
        
            for(size_t i = 0; i < size && elements; i++) {
                mem_pool.construct(&elements[i], args...);
            }

            return elements;
        }

        void destroy(T* ptr, size_t size) {
            for(size_t i = 0; i < size; i++) {
                mem_pool.destroy(ptr + i);
            }

            mem_pool.deallocate(ptr, size);
        }
    
    private:
        memory_pool_t<T> mem_pool;
    };

    template<typename T>
    bool operator==(const block_allocator_t<T>& _1, const block_allocator_t<T>& _2) {
        return _1.get_real_ptr() == _2.get_real_ptr();
    }

    template<typename T>
    bool operator!=(const block_allocator_t<T>& _1, const block_allocator_t<T>& _2) {
        return _1.get_real_ptr() != _2.get_real_ptr();
    }

    template<typename T>
    bool operator==(const memory_pool_t<T>& _1, const memory_pool_t<T>& _2) {
        return _1.get_allocators() == _2.get_allocators();
    }

    template<typename T>
    bool operator!=(const memory_pool_t<T>& _1, const memory_pool_t<T>& _2) {
        return _1.get_allocators() != _2.get_allocators();
    }
}