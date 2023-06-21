#pragma once

#include <stdint.h>
#include <malloc.h>
#include <bitset>
#include <functional>
#include <variant>
#include <assert.h>
#include <memory>
#include <cstring>

namespace ptm {
    constexpr auto system_alignment = sizeof(void*);

    typedef std::function<void(const char*)> log_func_t;

    inline log_func_t log = [](const char* msg) {
        printf("%s\n", msg);
    };
}