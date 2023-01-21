#pragma once

#include <cassert>
#include <iomanip>
#include <iostream>
#include <windows.h>
#include "types.hpp"
#include "table.hpp"

#define SIZE 512 * 1024

namespace qlog {

struct serializer {
        serializer() : offset(0)
        {
                handle = CreateFile("logs.qlb", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                assert(handle != INVALID_HANDLE_VALUE);

                buffer = reinterpret_cast<u8*>(VirtualAlloc(NULL, SIZE, MEM_COMMIT, PAGE_READWRITE));
        }

        ~serializer()
        {
                if (offset) flush();

                CloseHandle(handle);
                handle = INVALID_HANDLE_VALUE;
                
                handle = CreateFile("logs.qlb", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                assert(handle != INVALID_HANDLE_VALUE);

                DWORD written = 0;
                WriteFile(handle, &header, sizeof(file_header), &written, NULL);
                
                CloseHandle(handle);
                handle = INVALID_HANDLE_VALUE;
        }

        void write(const char* ptr, u64 size)
        {
                // TODO: add alignment using tellp() and seekp()
                // TODO: update file_pointer
                
                if (size + offset >= SIZE) {
                        flush();
                }
                
                memcpy(buffer + offset, ptr, size);
                offset += size;
        }

        void flush()
        {
                DWORD written = 0;
                WriteFile(handle, buffer, offset, &written, NULL);
                offset = 0;
        }

        void dump(const char* ptr, u64 size)
        {
                for (u64 i = 0; i < size; i++) {
                        std::cout << std::hex << std::setfill('0') << std::setw(2) << (i32)ptr[i] << " ";
                }
                std::cout << std::endl;
        }

        // void write(const char* ptr, u64 size, u64 pad);

        HANDLE handle;
        u64    offset;
        u8*    buffer;
        file_header header;
};

} // namespace qlog
