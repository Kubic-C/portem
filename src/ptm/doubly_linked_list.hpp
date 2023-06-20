#pragma once

#include "base.hpp"

namespace ptm {
    struct doubly_linked_list_element_t {
        doubly_linked_list_element_t()
            : prev(nullptr), next(nullptr) {}

        doubly_linked_list_element_t* prev;
        doubly_linked_list_element_t* next;
    };

    struct doubly_linked_list_header_t {
        doubly_linked_list_header_t()
            : first(nullptr), last(nullptr) {}

        doubly_linked_list_element_t* first;
        doubly_linked_list_element_t* last;

        bool is_empty() { return first == nullptr; }

        void push_front(doubly_linked_list_element_t* element);
        void push_back(doubly_linked_list_element_t* element);
        void remove_element(doubly_linked_list_element_t* element);
        void pop_front();
        void pop_back();

    protected:
        void assign_first(doubly_linked_list_element_t* element);
    };
}

namespace ptm {
    void doubly_linked_list_header_t::assign_first(doubly_linked_list_element_t* element) {
        first = element;
        last = element;
    }

    void doubly_linked_list_header_t::push_front(doubly_linked_list_element_t* element) {
        if(is_empty()) {
            assign_first(element);
        } else {
            first->prev = element;
            element->next = first;
            first = element; 
        }
    }

    void doubly_linked_list_header_t::push_back(doubly_linked_list_element_t* element) {
        if(is_empty()) {
            assign_first(element);
        } else {
            last->next = element;
            element->prev = last;
            last = element; 
        }
    }

    void doubly_linked_list_header_t::remove_element(doubly_linked_list_element_t* element) {
        if(element == first) {
            first = nullptr;
        }
        if(element == last) {
            last = nullptr;
        }

        if(element->prev) {
            element->prev->next = element->next;
        }
        if(element->next) {
            element->next->prev = element->prev;
        }
    }

    void doubly_linked_list_header_t::pop_front() {
        if(is_empty()) {
            return;
        } else {
            first = first->next;
        }
    }

    void doubly_linked_list_header_t::pop_back() {
        if(is_empty()) {
            return;
        } else {
            last = last->prev;
        }
    }
}