#pragma once

#include "base.hpp"

namespace syn {
    template<typename T, size_t capacity>
    class static_list {
    public:
        static_list();
        ~static_list();
        
        void push_back(const T& obj);
        void erase(size_t start, size_t end);
        void erase(size_t index);
        void pop_back();
        T&   emplace_back();

        void clear() {
            del(0, size);
            size = 0;
        }
    
        size_t get_size() {
            return size;
        }

        T* begin() {
            return data();
        }

        T* end() {
            return data() + size;
        }

        T* data() { 
            return list.data(); 
        }

        T* back() { 
            return &list[size - 1]; 
        }

        T* front() { 
            return &list[0];
        }

        T& operator[](size_t index) {
            return list[index];
        }
    private:
        void del(size_t start, size_t end) {
            for(size_t i = start; i < end; i++) {
                dtor(&list[i]);
            }
        }

        std::array<T, capacity> list;
        size_t size;
    };

    template<typename T, size_t capacity>
    static_list<T, capacity>::static_list() 
        : size(0) {
    }

    template<typename T, size_t capacity>
    static_list<T, capacity>::~static_list() {
        del(0, size);
    }

    template<typename T, size_t capacity>
    void static_list<T, capacity>::push_back(const T& obj) {
        assert(size + 1 <= capacity);
        
        size++;
        list[size - 1] = obj;
    }

    template<typename T, size_t capacity>
    T& static_list<T, capacity>::emplace_back() {
        assert(size + 1 <= capacity);

        size++;
        new(&list[size - 1])T();
        return *back();
    }

    template<typename T, size_t capacity>
    void static_list<T, capacity>::erase(size_t start, size_t end) {
        assert(start < end);
        assert(end < size);

        size = (size - end) + start;
        
        memcpy(data() + start, data() + size, sizeof(T) * size);
    }


    template<typename T, size_t capacity>
    void static_list<T, capacity>::erase(size_t index) {
        assert(index < size);

        uint32_t amount_after_index = size - (index + 1);

        // shifting all the elements down by 1
        memcpy(data() + index, data() + amount_after_index, sizeof(T) * amount_after_index);
        size--;    
    }

    template<typename T, size_t capacity>
    void static_list<T, capacity>::pop_back() {
        size--;
    }

}