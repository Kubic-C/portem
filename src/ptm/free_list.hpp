#pragma once

#include "base.hpp"

namespace ptm {

    // This code is from a stack overflow post, which honestly made a really
    // clean implementation (https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det/48330314#48330314) Full credit should go the author, but the account has since been deleted on stack overflow
    //
    // Provides an indexed free list with constant-time removals from anywhere
    // in the list without invalidating indices. T must be trivially constructible 
    // and destructible.
    template <class T>
    class free_list_t {
    public:
        /// Creates a new free list.
        free_list_t();

        /// Inserts an element to the free list and returns an index to it.
        int insert(const T& element);

        // Removes the nth element from the free list.
        void erase(int n);

        // Removes all elements from the free list.
        void clear();

        // Returns the range of valid indices.
        int range() const;

        // Reserve additional space within the vector to prevent
        // additional malloc calls
        void reserve(size_t size);

        // Returns the nth element.
        T& operator[](int n);

        // Returns the nth element.
        const T& operator[](int n) const;

    private:
        union free_element_t {
            T element;
            int next;
        };

        std::vector<free_element_t> data;
        int first_free;
    };

    template <class T>
    free_list_t<T>::free_list_t()
        : first_free(-1) {
    }

    template <class T>
    int free_list_t<T>::insert(const T& element) {
        if (first_free != -1) {
            const int index = first_free;
            first_free = data[first_free].next;
            data[index].element = element;
            return index;
        } else {
            free_element_t fe;
            fe.element = element;
            data.push_back(fe);
            return static_cast<int>(data.size() - 1);
        }
    }

    template <class T>
    void free_list_t<T>::erase(int n) {
        data[n].next = first_free;
        first_free = n;
    }

    template <class T>
    void free_list_t<T>::clear() {
        data.clear();
        first_free = -1;
    }

    template <class T>
    int free_list_t<T>::range() const {
        return static_cast<int>(data.size());
    }

    template<class T>
    void free_list_t<T>::reserve(size_t size) {
        data.reserve(size);
    }

    template <class T>
    T& free_list_t<T>::operator[](int n) {
        return data[n].element;
    }

    template <class T>
    const T& free_list_t<T>::operator[](int n) const {
        return data[n].element;
    }
}