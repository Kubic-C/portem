#pragma once

#include <stdint.h>
#include <malloc.h>
#include <bitset>
#include <functional>
#include <assert.h>

namespace ptm {
    typedef std::function<void(const char*)> log_func_t;

    inline log_func_t log = [](const char* msg) {
        printf("%s\n", msg);
    };
}