#pragma once

#include <cstdint>
#include <tuple>

namespace qlog {

enum log_level : uint32_t {
        QLOG_INFO = 0,
        QLOG_DEBUG,
        QLOG_WARNING,
        QLOG_ERROR,
        QLOG_TRACE,
        QLOG_MAX
};

enum log_data_type : uint32_t {
        QLOG_DATA_TYPE_CHAR = 0,
        QLOG_DATA_TYPE_INT8,
        QLOG_DATA_TYPE_INT16,
        QLOG_DATA_TYPE_INT32,
        QLOG_DATA_TYPE_INT64,
        QLOG_DATA_TYPE_UINT8,
        QLOG_DATA_TYPE_UINT16,
        QLOG_DATA_TYPE_UINT32,
        QLOG_DATA_TYPE_UINT64,
        QLOG_DATA_TYPE_FLOAT,
        QLOG_DATA_TYPE_DOUBLE,
        // QLOG_DATA_TYPE_ARRAY,
};

template <typename T>
constexpr size_t variadic_argument_count(const T& t)
{
        return 1;
}

template <typename T, typename... A>
constexpr size_t variadic_argument_count(const T& t, const A&... a)
{
        return 1 + variadic_argument_count(a...);
}

struct static_table {
        uint32_t id;
        uint32_t level;
        uint32_t line;
        uint32_t str_len;
        uint32_t arg_count;
};

struct dynamic_table {
        uint32_t id;
        // args data
};

template <size_t N>
struct cstring {
        constexpr cstring(const char (&s)[N]) noexcept
        {
                for (size_t i = 0; i < N; i++) _raw[i] = s[i];
        }

        char operator[](const size_t i)
        {
                return _raw[i];
        }
        char& operator[](const size_t i) const
        {
                return _raw[i];
        }

        template <size_t M>
        constexpr size_t contains(const cstring<M>& s) noexcept
        {
                size_t count = 0;
                for (size_t i = 0; i < (N + 1) - s.len; i++) {
                        if (_raw[i] != s[0]) continue;
                        for (size_t j = 0; j < s.len; j++) {
                                if (_raw[i + j] != s[j]) {
                                        count--;
                                        break;
                                }
                        }
                        count++;
                }
                return count;
        }

        size_t len = N;
        char   _raw[N];
};

template <size_t N>
struct argument_types_table {
        template <typename... A>
        constexpr argument_types_table(const A... a) noexcept
        {
                construct_table(0, a...);
        }

        template <typename T>
        constexpr void contruct_table(size_t i, const T& t)
        {
                if (is_type_char<T>::value) _types[i] = QLOG_DATA_TYPE_CHAR;
                else if (is_type_int8<T>::value) _types[i] = QLOG_DATA_TYPE_INT8;
                else if (is_type_int16<T>::value) _types[i] = QLOG_DATA_TYPE_INT16;
                else if (is_type_int32<T>::value) _types[i] = QLOG_DATA_TYPE_INT32;
                else if (is_type_int64<T>::value) _types[i] = QLOG_DATA_TYPE_INT64;
                else if (is_type_uint8<T>::value) _types[i] = QLOG_DATA_TYPE_UINT8;
                else if (is_type_uint16<T>::value) _types[i] = QLOG_DATA_TYPE_UINT16;
                else if (is_type_uint32<T>::value) _types[i] = QLOG_DATA_TYPE_UINT32;
                else if (is_type_uint64<T>::value) _types[i] = QLOG_DATA_TYPE_UINT64;
                else if (is_type_float<T>::value) _types[i] = QLOG_DATA_TYPE_FLOAT;
                else if (is_type_double<T>::value) _types[i] = QLOG_DATA_TYPE_DOUBLE;
                else exit(1);
        }
        
        template <typename T, typename... A>
        constexpr void contruct_table(size_t i, const T& t, const A&... a)
        {
                if (is_type_char<T>::value) _types[i] = QLOG_DATA_TYPE_CHAR;
                else if (is_type_int8<T>::value) _types[i] = QLOG_DATA_TYPE_INT8;
                else if (is_type_int16<T>::value) _types[i] = QLOG_DATA_TYPE_INT16;
                else if (is_type_int32<T>::value) _types[i] = QLOG_DATA_TYPE_INT32;
                else if (is_type_int64<T>::value) _types[i] = QLOG_DATA_TYPE_INT64;
                else if (is_type_uint8<T>::value) _types[i] = QLOG_DATA_TYPE_UINT8;
                else if (is_type_uint16<T>::value) _types[i] = QLOG_DATA_TYPE_UINT16;
                else if (is_type_uint32<T>::value) _types[i] = QLOG_DATA_TYPE_UINT32;
                else if (is_type_uint64<T>::value) _types[i] = QLOG_DATA_TYPE_UINT64;
                else if (is_type_float<T>::value) _types[i] = QLOG_DATA_TYPE_FLOAT;
                else if (is_type_double<T>::value) _types[i] = QLOG_DATA_TYPE_DOUBLE;
                else exit(1);
                construct_table(i + 1, a...);
        }

        uint32_t _types[N];
};

// creates tables that are then pass on to serializer
struct writer {
        constexpr writer();

        template <size_t N, size_t M, typename... A>
        static constexpr size_t add_static_table(const uint32_t level, const cstring<N>& str, const cstring<M>& file, const size_t line,
                                                 const A&&... a) noexcept
        {
                size_t id = global_unique_log_count;
                global_unique_log_count++;

                const static_table table = {id, level, str.len, arg_count};
                serializer.write(table);

                // ???
                serializer.write(str._raw);

                // construct argument type table
                constexpr size_t                      arg_count = variadic_argument_count(a...);
                const argument_types_table<arg_count> arg_types = {a...};
                serializer.write(arg_types);

                return id;
        }

        template <typename... A>
        static constexpr void add_dynamic_table(const uint32_t arg_count, const A&&... a) noexcept;
};

#define qlog(level, str, ...)                                                            \
        {                                                                                \
                static_assert(0 <= level && level < QLOG_MAX);                           \
                constexpr cstring s = {str};                                             \
                static_assert(s.contains("{}") == variadic_argument_count(__VA_ARGS__)); \
                                                                                         \
                constexpr arg_count = count_args(__VA_ARGS__);                           \
                static size_t id = 0;                                                    \
                if (id == 0) {                                                           \
                        id = writer.add_static_table(...);                               \
                }                                                                        \
                writer.add_dynamic_table(...);                                           \
        }

/*
        EXAMPLE:
         qlog(QLOG_DEBUG, "incorrect input value, {}", image_queue.error);

        OUTPUT:
         "[debug] main.cpp:45 --- incorrect input value, image_queue.error 545\n"
 */

} // namespace qlog