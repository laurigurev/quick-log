#pragma once

#include "types.hpp"
#include <type_traits>

namespace qlog {

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
        QLOG_DATA_TYPE_OTHER
};

template <u32 N>
struct type_array {
        template <typename... A>
        type_array(A... a)
        {
                construct_array(0, a...);
        }

        template <typename T, typename... A>
        constexpr void construct_array(u32 i, T t)
        {
                if (std::is_same<T, char>::value) types[i] = QLOG_DATA_TYPE_CHAR;
                else if (std::is_same<T, i8>::value) types[i] = QLOG_DATA_TYPE_INT8;
                else if (std::is_same<T, i16>::value) types[i] = QLOG_DATA_TYPE_INT16;
                else if (std::is_same<T, i32>::value) types[i] = QLOG_DATA_TYPE_INT32;
                else if (std::is_same<T, i64>::value) types[i] = QLOG_DATA_TYPE_INT64;
                else if (std::is_same<T, u8>::value) types[i] = QLOG_DATA_TYPE_UINT8;
                else if (std::is_same<T, u16>::value) types[i] = QLOG_DATA_TYPE_UINT16;
                else if (std::is_same<T, u32>::value) types[i] = QLOG_DATA_TYPE_UINT32;
                else if (std::is_same<T, u64>::value) types[i] = QLOG_DATA_TYPE_UINT64;
                else if (std::is_same<T, f32>::value) types[i] = QLOG_DATA_TYPE_FLOAT;
                else if (std::is_same<T, d64>::value) types[i] = QLOG_DATA_TYPE_DOUBLE;
                else types[i] = QLOG_DATA_TYPE_OTHER;
        }

        template <typename T, typename... A>
        constexpr void construct_array(u32 i, T t, A... a)
        {
                if (std::is_same<T, char>::value) types[i] = QLOG_DATA_TYPE_CHAR;
                else if (std::is_same<T, i8>::value) types[i] = QLOG_DATA_TYPE_INT8;
                else if (std::is_same<T, i16>::value) types[i] = QLOG_DATA_TYPE_INT16;
                else if (std::is_same<T, i32>::value) types[i] = QLOG_DATA_TYPE_INT32;
                else if (std::is_same<T, i64>::value) types[i] = QLOG_DATA_TYPE_INT64;
                else if (std::is_same<T, u8>::value) types[i] = QLOG_DATA_TYPE_UINT8;
                else if (std::is_same<T, u16>::value) types[i] = QLOG_DATA_TYPE_UINT16;
                else if (std::is_same<T, u32>::value) types[i] = QLOG_DATA_TYPE_UINT32;
                else if (std::is_same<T, u64>::value) types[i] = QLOG_DATA_TYPE_UINT64;
                else if (std::is_same<T, f32>::value) types[i] = QLOG_DATA_TYPE_FLOAT;
                else if (std::is_same<T, d64>::value) types[i] = QLOG_DATA_TYPE_DOUBLE;
                else types[i] = QLOG_DATA_TYPE_OTHER;
                construct_array(i + 1, a...);
        }

        constexpr u32 size() const
        {
                return N;
        }

        char types[N];
};

// copied from binlog
template <typename... T>
constexpr std::integral_constant<std::size_t, sizeof...(T)> count_arguments(T&&...)
{
        return {};
} // Implementation should be omitted but cannot be on MSVC

} // namespace qlog
