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

namespace ptm {
    constexpr auto system_alignment = sizeof(void*);

    typedef std::function<void(const char*)> log_func_t;

    // if you see any of these values on a variable
    // when debugging, chances are the: the variable is not initialized
    constexpr float blatent_f = std::numeric_limits<float>::max();
    constexpr float blatent_i32 = std::numeric_limits<int32_t>::max();
    constexpr float blatent_u32 = std::numeric_limits<uint32_t>::max();
    constexpr float blatent_u8 = std::numeric_limits<uint8_t>::max();

    inline log_func_t log = [](const char* msg) {
        printf("%s\n", msg);
    };
}