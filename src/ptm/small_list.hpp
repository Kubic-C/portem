#pragma once

#include "base.hpp"

namespace ptm {
    // a list for holding simple data types or structs
    // use only trivial objects
    template<typename T, size_t max = 128>
    class small_list_t {
        using stack_buffer_t = std::array<T, max>;
        using heap_buffer_t  = std::vector<T>;

    public:
        small_list_t() {} // variants by default, construct the first alternative

        void push_back(const T& element) {
            if(use_heap) {
                heap_buffer().push_back(element);
            } else if(count + 1 > max) {
                // copy the contents of the stack buffer over 
                // to a new heap buffer
                std::vector<T> new_buffer = {};
                // reserve enough space for all previous elements
                // and the new one
                new_buffer.reserve(max + 1);
                new_buffer.insert(new_buffer.begin(), stack_buffer().begin(), stack_buffer().end());
                
                // set the variant as a heap buffer
                use_heap = true; 
                buffer.template emplace<1>(new_buffer);
                heap_buffer().push_back(element);
            } else {
                stack_buffer()[count] = element;
            }

            // cant forget this
            count++;
        }

        void pop_back() {
            if(use_heap) {
                heap_buffer().pop_back();
            } 

            count--;
        }

        T& operator[](size_t i) {
            assert(i < count);

            if(use_heap)
                return heap_buffer()[i];
            else 
                return stack_buffer()[i];
        }

    private:
        stack_buffer_t& stack_buffer() { return std::get<0>(buffer); }
        heap_buffer_t& heap_buffer() { return std::get<1>(buffer); }

        uint32_t count    = 0;
        bool     use_heap = false;

        // an index into the variant that points
        // to either the stack based array or the vector
        std::variant<stack_buffer_t, heap_buffer_t> buffer;
    };
}