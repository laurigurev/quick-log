#pragma once

#include "args.hpp"
#include "cstring.hpp"
#include "serializer.hpp"
#include "table.hpp"

namespace qlog {

struct writer {
        writer() : unique_log_count(0) {}

        template <u32 N, u32 M, typename... A>
        constexpr u16 push_static(const u32 level, const cstring<N>& file, const u32 line, const cstring<M>& format, const A... a)
        {
                const u16 id = unique_log_count;
                unique_log_count++;

                const table_header header = {id, QLOG_TABLE_TYPE_STATIC};
                constexpr u32      arg_count = sizeof...(a);
                const u32          offset0 = ser.file_pointer + sizeof(static_table);
                const u32          offset1 = offset0 + file.len;
                const u32          offset2 = offset1 + format.len;
                const static_table table = {header, level, file.len, offset0, line, format.len, offset1, arg_count, offset2};

                const type_array<arg_count> arg_t = {a...};

                ser.write(reinterpret_cast<const char*>(&table), sizeof(static_table));
                ser.write(file._raw, file.len);
                ser.write(format._raw, format.len);
                ser.write(reinterpret_cast<const char*>(&arg_t), sizeof(arg_t));

                return id;
        }

        template <typename... A>
        constexpr void push_dynamic(const u16 id, A... a)
        {
                const table_header  header = {id, QLOG_TABLE_TYPE_DYNAMIC};
                const u32           offset0 = ser.file_pointer + sizeof(dynamic_table);
                const dynamic_table table = {header, offset0};
                std::tuple<A...>    tuple = {a...};

                ser.write(reinterpret_cast<const char*>(&table), sizeof(dynamic_table));
                ser.write(reinterpret_cast<const char*>(&tuple), sizeof(tuple));
        }

        u64        unique_log_count;
        serializer ser;
};

} // namespace qlog