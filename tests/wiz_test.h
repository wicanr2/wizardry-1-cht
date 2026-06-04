#pragma once
#include <cstdio>
#include <cstdlib>

#define WIZ_CHECK(cond) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            std::exit(1); \
        } \
    } while (0)

#define WIZ_CHECK_EQ(a, b) \
    do { \
        auto _a = (a); auto _b = (b); \
        if (!(_a == _b)) { \
            std::fprintf(stderr, "FAIL %s:%d  %s != %s\n", __FILE__, __LINE__, #a, #b); \
            std::exit(1); \
        } \
    } while (0)
