#pragma once

namespace ptm {
        template<typename T>
    class allocator_t {
    public:
      typedef T              _Tp;
      typedef _Tp            value_type;
      typedef size_t         size_type;
      typedef ptrdiff_t      difference_type;
      typedef _Tp*           pointer;
      typedef const _Tp*     const_pointer;
      typedef _Tp&           reference;
      typedef const _Tp&     const_reference;
      typedef allocator_t<T> allocator_type;

      template <class U> 
      struct rebind { typedef allocator_t<U> other; };

    public:
        virtual T* address(T& x) {
            return &x;
        }

        virtual void construct(T* ptr, const T& tp) {
            *ptr = tp;
        }
    
        template<typename ... params>
        void construct(T* ptr, params&& ... args) {
            new(ptr)T(args...);
        }

        virtual void destroy(T* ptr) {
            ptr->~T();
        }

        virtual T* allocate(size_t n, const void* hint = 0) {
            return (T*)calloc(n, sizeof(T));
        }   

        virtual void deallocate(T* ptr, size_t n) {
            free(ptr);
        }

        virtual bool operator==(const allocator_t& alloc) {
            return true;
        }

        virtual bool operator!=(const allocator_t& alloc) {
            return true;
        }
    };
}