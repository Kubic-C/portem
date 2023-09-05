#pragma once

#include <stdint.h>
#include <malloc.h>
#include <bitset>
#include <functional>
#include <vector>
#include <variant>
#include <assert.h>
#include <limits>
#include <memory>
#include <cstring>
#include <map>
#include <typeindex>

namespace ptm {
    constexpr auto system_alignment = sizeof(void*);

    typedef std::function<void(const char*)> log_func_t;

    // if you see any of these values on a variable
    // when debugging, chances are the: the variable is not initialized
    constexpr float    blatent_f   = std::numeric_limits<float>::max();
    constexpr int32_t  blatent_i32 = std::numeric_limits<int32_t>::max();
    constexpr uint8_t  blatent_u8  = std::numeric_limits<uint8_t>::max();
    constexpr uint32_t blatent_u32 = std::numeric_limits<uint32_t>::max();

    inline log_func_t log = [](const char* msg) {
        printf("%s\n", msg);
    };

    template<typename T>
    void zero(T* dst) {
        memset(dst, 0, sizeof(T));
    }

    template<typename T>
    T null_object() {
        T value;

        memset(&value, 0, sizeof(T));
        return value;
    }

    template<typename T>
    T probably_an_invalid_object() {
        T value;

        memset(&value, INT_MAX, sizeof(T));
        return value;
    }

    template<typename T>
    bool bytecmp(const T& _1, const T& _2) {
        return memcmp(&_1, &_2, sizeof(T));
    }
}