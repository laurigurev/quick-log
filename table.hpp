#pragma once

#include "types.hpp"

namespace qlog {

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
