#pragma once

#include "args.hpp"
#include "table.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <windows.h>

namespace qlog {

namespace des {

struct static_table {
        qlog::static_table table;
        char*              file;
        char*              format;
        char*              argts;
        u64                args_size;
};

struct mem_arena {
        mem_arena(const u64 size)
        {
                start = reinterpret_cast<char*>(malloc(size));
                pointer = start;
                end = start + size;
        }

        char* push(const u64 n)
        {
                assert(pointer + n < end);

                char* ret = pointer;
                pointer += n;
                return ret;
        }

        char* start;
        char* pointer;
        char* end;
};

} // namespace des

struct deserializer {
        deserializer() : file_pointer(0), ma(1024 * 1024)
        {
                handle = CreateFile("logs.qlb", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                assert(handle != INVALID_HANDLE_VALUE);

                ReadFile(handle, &header, sizeof(file_header), NULL, NULL);
                // assert signature

                static_tables.reserve(header.static_table_count);

                while (header.log_count--) {
                        table_header tab_head;
                        ReadFile(handle, &tab_head, sizeof(table_header), NULL, NULL);
                        // TODO: set file pointer back by sizeof(table_header)
                        static const i64 off = - sizeof(table_header);
                        file_pointer = SetFilePointer(handle, off, NULL, FILE_CURRENT);

                        if (tab_head.table_type == QLOG_TABLE_TYPE_STATIC) parse_static(tab_head.id);
                        else if (tab_head.table_type == QLOG_TABLE_TYPE_DYNAMIC) parse_dynamic(tab_head.id);
                        else assert(0);
                }
                
                CloseHandle(handle);
                handle = INVALID_HANDLE_VALUE;
        }

        void parse_static(u16 id)
        {
                static static_table statab;
                ReadFile(handle, &statab, sizeof(static_table), NULL, NULL);

                static des::static_table table;
                table.table = statab;
                table.file = ma.push(statab.file_len);
                table.format = ma.push(statab.format_len);
                table.argts = ma.push(statab.arg_count);
                // This should work on x64, because everything is 8 byte aligned
                table.args_size = statab.arg_count * 8;

                ReadFile(handle, table.file, statab.file_len, NULL, NULL);
                ReadFile(handle, table.format, statab.format_len, NULL, NULL);
                ReadFile(handle, table.argts, statab.arg_count, NULL, NULL);

                static_tables[id] = table;
        }

        void parse_dynamic(u16 id)
        {
                static const char* levels[] = {"[info] ", "[debug] ", "[warn] ", "[error] ", "[trace] "};
                static std::string format, token;
                static const std::string delimiter = "{}";

                // printf("deserializer::parse_dynamic(%i), file_pointer %llu\n", id, file_pointer);
                
                static dynamic_table dyntab;
                ReadFile(handle, &dyntab, sizeof(dynamic_table), NULL, NULL);
                
                const des::static_table& table = static_tables[id];
                format = table.format;

                char* ma_prev = ma.pointer;
                char* raw = ma.push(table.args_size);
                
                ReadFile(handle, raw, table.args_size, NULL, NULL);
                
                std::cout << levels[table.table.level] << table.file << ":" << table.table.line << " --- ";
                
                u64   offset = 0;
                u64 i = 0;
                u64 pos = 0;

                while ((pos = format.find(delimiter)) != std::string::npos) {
                        token = format.substr(0, pos);
                        std::cout << token;
                        format.erase(0, pos + delimiter.length());
                        
                        switch (table.argts[i]) {
                        case QLOG_DATA_TYPE_CHAR: {
                                char* tmp = raw + offset;
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_INT8: {
                                i8* tmp = reinterpret_cast<i8*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_INT16: {
                                i16* tmp = reinterpret_cast<i16*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_INT32: {
                                i32* tmp = reinterpret_cast<i32*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_INT64: {
                                i64* tmp = reinterpret_cast<i64*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_UINT8: {

                                u8* tmp = reinterpret_cast<u8*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_UINT16: {
                                u16* tmp = reinterpret_cast<u16*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_UINT32: {
                                u32* tmp = reinterpret_cast<u32*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_UINT64: {
                                u64* tmp = reinterpret_cast<u64*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_FLOAT: {
                                float* tmp = reinterpret_cast<float*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_DOUBLE: {
                                double* tmp = reinterpret_cast<double*>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        case QLOG_DATA_TYPE_STRING_LITERAL: {
                                const char** tmp = reinterpret_cast<const char**>(raw + offset);
                                offset += 8;
                                std::cout << *tmp;
                                break;
                        }
                        default:
                                assert(0 && "unsupported data type");
                        }

                        i++;
                }

                if (table.table.arg_count == 0) std::cout << format;

                std::cout << '\n';
                ma.pointer = ma_prev;
        }

        HANDLE                         handle;
        u64                            file_pointer;
        file_header                    header;
        std::vector<des::static_table> static_tables;
        des::mem_arena                 ma;
};

} // namespace qlog
