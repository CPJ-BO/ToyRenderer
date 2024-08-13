#pragma once

#include <cstdio>
#include <stdexcept>

// #define LOG_DEBUG(fmt, ...) do { \
//     fprintf(stderr, "[%s:%s:%d] " fmt "\n",  __FILE__, __func__, __LINE__, __VA_ARGS__);  \
// } while (0)

#define LOG_DEBUG(fmt, ...) do { \
    fprintf(stderr, fmt "\n", __VA_ARGS__);  \
} while (0)

#define LOG_FATAL(fmt, ...) do { \
    fprintf(stderr, "[%s:%s:%d] " fmt "\n", __FILE__, __func__, __LINE__, __VA_ARGS__);  \
    throw std::runtime_error(""); \
} while (0)
