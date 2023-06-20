#pragma once

#include "base.hpp"

namespace ptm {
    struct doubly_linked_list_element_t {
        doubly_linked_list_element_t()
            : prev(nullptr), next(nullptr) {}

        virtual ~doubly_linked_list_element_t() = default;

        doubly_linked_list_element_t* prev;
        doubly_linked_list_element_t* next;
    };

    template<typename T>
    struct doubly_linked_list_header_t {
        doubly_linked_list_header_t()
            : first(nullptr), last(nullptr) {}

        doubly_linked_list_element_t* first;
        doubly_linked_list_element_t* last;

        bool is_empty() { return first == nullptr; }

        void push_front(T* element);
        void push_back(T* element);
        void remove_element(T* element);
        void pop_front();
        void pop_back();

    protected:
        void assign_first(doubly_linked_list_element_t* element);
    };
}

namespace ptm {
    template<typename T>
    void doubly_linked_list_header_t<T>::assign_first(doubly_linked_list_element_t* element) {
        first = element;
        last  = element;
    }

    template<typename T>
    void doubly_linked_list_header_t<T>::push_front(T* element) {
        doubly_linked_list_element_t* list_element = dynamic_cast<doubly_linked_list_element_t*>(element);  

        if(is_empty()) {
            assign_first(list_element);
        } else {
            first->prev = list_element;
            list_element->next = first;
            first = list_element; 
        }
    }

    template<typename T>
    void doubly_linked_list_header_t<T>::push_back(T* element) {
        doubly_linked_list_element_t* list_element = dynamic_cast<doubly_linked_list_element_t*>(element);  

        if(is_empty()) {
            assign_first(list_element);
        } else {
            last->next = list_element;
            list_element->prev = last;
            last = list_element; 
        }
    }

    template<typename T>
    void doubly_linked_list_header_t<T>::remove_element(T* element) {
        doubly_linked_list_element_t* list_element = dynamic_cast<doubly_linked_list_element_t*>(element);  

        if(list_element == first) {
            first = list_element->next;
        }
        if(list_element == last) {
            last = list_element->prev;
        }

        if(list_element->prev) {
            list_element->prev->next = list_element->next;
        }
        if(list_element->next) {
            list_element->next->prev = list_element->prev;
        }

        list_element->prev = nullptr;
        list_element->next = nullptr;
    }

    template<typename T>
    void doubly_linked_list_header_t<T>::pop_front() {
        if(is_empty()) {
            return;
        } else {
            first = first->next;

            first->prev = nullptr;
            first->next = nullptr;
        }
    }
    template<typename T>
    void doubly_linked_list_header_t<T>::pop_back() {
        if(is_empty()) {
            return;
        } else {
            last = last->prev;

            last->prev = nullptr;
            last->next = nullptr;
        }
    }
}