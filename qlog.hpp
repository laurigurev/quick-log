#pragma once

#include "types.hpp"
#include "writer.hpp"

enum log_level : u32 {
        QLOG_INFO = 0,
        QLOG_DEBUG,
        QLOG_WARNING,
        QLOG_ERROR,
        QLOG_TRACE,
        QLOG_MAX
};

static qlog::writer WRITER;

#define QLOG(level, str, ...)                                                                    \
        {                                                                                        \
                static size_t id = 0;                                                            \
                                                                                                 \
                static_assert(0 <= level && level < QLOG_MAX);                                   \
                constexpr qlog::cstring format = {str};                                          \
                constexpr qlog::cstring specifier = {"{}"};                                      \
                static_assert(format.contains(specifier) == qlog::count_arguments(__VA_ARGS__)); \
                                                                                                 \
                if (id == 0) {                                                                   \
                        constexpr qlog::cstring file = {__FILE__};                               \
                        id = WRITER.push_static(level, file, __LINE__, format, __VA_ARGS__);     \
                }                                                                                \
                WRITER.push_dynamic(id, __VA_ARGS__);                                            \
        }

/*
        EXAMPLE:
         qlog(QLOG_DEBUG, "incorrect input value, {}", image_queue.error);

        OUTPUT:
         "[debug] main.cpp:45 --- incorrect input value, 545\n"
 */
