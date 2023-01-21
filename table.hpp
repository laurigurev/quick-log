#pragma once

#include "types.hpp"

namespace qlog {

struct file_header {
        file_header() : signature{'.', 'q', 'l', 'b'}, padding(0), static_table_count(0), log_count(0) {}
        file_header(const u64 s, const u64 l) : signature{'.', 'q', 'l', 'b'}, padding(0), static_table_count(s), log_count(l) {}
        
        char signature[4];
        u32 padding;
        u64 static_table_count;
        u64 log_count;
};

enum table_type : u16 {
        QLOG_TABLE_TYPE_STATIC = 0,
        QLOG_TABLE_TYPE_DYNAMIC
};

struct table_header {
        u16 id;
        u16 table_type;
};

struct static_table {
        table_header header;
        u32          level;
        u32          file_len;
        u32          line;
        u32          format_len;
        u32          arg_count;
        // table size can be inferred
        // offsets can be inferred
};

struct dynamic_table {
        table_header header;
        // we infer arguments_len from arg_count AND arg_types
        // table size can be inferred
};

// ... | static_table | file | format | arg_t | ...
// ... | dynamic_table | args | ... | dynamic_table | args | ... 

} // namespace qlog
