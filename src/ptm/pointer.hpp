#pragma once

#include "base.hpp"

namespace ptm {
    template<typename T>
    class ptr_t {
    public:
        template<typename U>
        static constexpr bool castable = std::is_convertible_v<U*, T*> || std::is_convertible_v<T*, U*>;
    
        ptr_t() {}
                                                                 /* allows upcasting vvv */
        template<typename U, bool can_convert = castable<U>>
        ptr_t(ptr_t<U> other) {
            static_assert(can_convert == true);

            object = static_cast<T*>(other.get());
        }

        ptr_t(T* object)
            : object(object) {}

        bool is_null() const {
            return object == nullptr;
        }

        void set(T* new_object) {
            object = new_object;
        }

        T* get() const {
            return object;
        }

        T& operator*() {
            return *object;
        }

        T* operator->() const {
            return object;
        }

        template<typename U, bool can_convert = castable<U>>
        operator U*() {
            return cast<U, can_convert>();
        }

        template<typename U, bool can_convert = castable<U>>
        U* cast() {
            static_assert(can_convert == true);
            
            return static_cast<U*>(object);    
        }

        bool operator==(const ptr_t<T>& other) {
            return (uint8_t*)object == (uint8_t*)other.get();
        }

    private:
        T* object; 
    }; 
}