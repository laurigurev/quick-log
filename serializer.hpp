#pragma once

#include <fstream>
#include "types.hpp"

namespace qlog {

struct serializer {
        serializer() : file_pointer(0)
        {
               out = std::ofstream("logs.qlb", std::ios::out); 
        }
        
        void write(const char* ptr, u64 size)
        {
                // TODO: add alignment using tellp() and seekp()
                // TODO: update file_pointer
                out.write(ptr, size);
        }

        void write(const char* ptr, u64 size, u64 pad);

        std::ofstream out;
        u64 file_pointer;
};

} // namespace qlog
