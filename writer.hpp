#pragma once

#include "args.hpp"
#include "cstring.hpp"
#include "serializer.hpp"
#include "deserializer.hpp"
#include "table.hpp"
#include "tuple.hpp"

namespace qlog {

struct writer {
        writer() : unique_log_count(0), log_count(0)
        {
                const file_header header = {0, 0};
                ser.write(reinterpret_cast<const char*>(&header), sizeof(file_header));
        }

        ~writer() 
        {
                const file_header header = {unique_log_count, log_count};
                ser.header = header;

                ser.clean_up();

                deserializer des = {};
        }

        template <u32 N, u32 M, typename... A>
        constexpr u16 push_static(const u32 level, const cstring<N>& file, const u32 line, const cstring<M>& format, const A... a)
        {
                const u16 id = unique_log_count;
                unique_log_count++;
                log_count++;

                const table_header          header = {id, QLOG_TABLE_TYPE_STATIC};
                constexpr u32               arg_count = sizeof...(a);
                const static_table          table = {header, level, file.len, line, format.len, arg_count};
                const type_array<arg_count> arg_t = {a...};

                ser.write(reinterpret_cast<const char*>(&table), sizeof(static_table));
                ser.write(file._raw, file.len);
                ser.write(format._raw, format.len);
                ser.write(arg_t.types, sizeof(arg_t));

                return id;
        }

        template <u32 N, u32 M, typename... A>
        constexpr u16 push_static(const u32 level, const cstring<N>& file, const u32 line, const cstring<M>& format)
        {
                const u16 id = unique_log_count;
                unique_log_count++;
                log_count++;

                const table_header header = {id, QLOG_TABLE_TYPE_STATIC};
                const static_table table = {header, level, file.len, line, format.len, 0};

                ser.write(reinterpret_cast<const char*>(&table), sizeof(static_table));
                ser.write(file._raw, file.len);
                ser.write(format._raw, format.len);

                return id;
        }

        template <typename... A>
        constexpr void push_dynamic(const u16 id, const A&... a)
        {
                log_count++;
                
                const table_header  header = {id, QLOG_TABLE_TYPE_DYNAMIC};
                const dynamic_table table = {header};
                const tuple<A...>   tuple = {a...};

                // printf("qlog::writer::push_dynamic(...), sizeof(tuple) %llu\n", sizeof(tuple));

                ser.write(reinterpret_cast<const char*>(&table), sizeof(dynamic_table));
                ser.write(reinterpret_cast<const char*>(&tuple), sizeof(tuple));
        }

        template <typename... A>
        constexpr void push_dynamic(const u16 id)
        {
                log_count++;
                
                const table_header  header = {id, QLOG_TABLE_TYPE_DYNAMIC};
                const dynamic_table table = {header};

                // printf("qlog::writer::push_dynamic(...), sizeof(tuple) %llu\n", sizeof(tuple));

                ser.write(reinterpret_cast<const char*>(&table), sizeof(dynamic_table));
        }

        u64        unique_log_count;
        u64 log_count;
        serializer ser;
};

} // namespace qlog