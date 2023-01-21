#pragma once

#include <fstream>
#include <iostream>
#include <iomanip>
#include "types.hpp"

namespace qlog {

struct serializer {
        serializer() : file_pointer(0)
        {
                std::ios::sync_with_stdio(false);
                out.rdbuf()->pubsetbuf(buffer, size);
                out.open("logs.qlb", std::ios::binary);
        }

        ~serializer()
        {
                out.close();
        }
        
        void write(const char* ptr, u64 size)
        {
                // TODO: add alignment using tellp() and seekp()
                // TODO: update file_pointer
                out.write(ptr, size);
        }

        void dump(const char* ptr, u64 size) 
        {
                for (u64 i = 0; i < size; i++) {
                        std::cout << std::hex << std::setfill('0') << std::setw(2) << (i32)ptr[i] << " ";
                }
                std::cout << std::endl;
        }

        void write(const char* ptr, u64 size, u64 pad);

        std::ofstream out;
        u64 file_pointer;
        static const u64 size = 256 * 1024;
        char buffer[size];
};

} // namespace qlog
