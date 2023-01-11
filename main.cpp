#include <cassert>
#include <cstdint>
#include <cstdio>
#include <windows.h>

template <size_t N>
struct hard_buffer {
        uint8_t data[N];
        size_t  index;

        hard_buffer() : index(0) {}

        template <typename T>
        bool push(const T& t, const size_t& len, size_t& offset)
        {
                bool   ret = true;
                size_t delta = len - offset;

                if (index + delta >= N) {
                        delta = N - index - 1;
                        ret = false;
                }

                memcpy(data + index, &t + offset, delta);
                index += delta;
                offset += delta;

                return ret;
        }

        bool push(const char* str, const size_t& len, size_t& offset)
        {
                bool   ret = true;
                size_t delta = len - offset;

                if (index + delta >= N) {
                        delta = N - index - 1;
                        ret = false;
                }

                memcpy(data + index, str + offset, delta);
                index += delta;
                offset += delta;

                return ret;
        }
};

struct streamer {
        hard_buffer<1u << 16> buffer;
        HANDLE                handle;

        streamer(const char* file)
        {
                handle = CreateFile(file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        ~streamer()
        {
                write();
                CloseHandle(handle);
        }

        void write()
        {
                // disable for testing
                // WriteFile(handle, buffer.data, buffer.index, NULL, NULL);
                buffer.index = 0;
        }

        void fprint(const char* str) noexcept
        {
                size_t       offset = 0;
                const size_t len = strlen(str);
                while (!buffer.push(str, len, offset)) write();
        }

        template <typename T>
        constexpr void fprint(const T& t) noexcept
        {
                size_t       offset = 0;
                const size_t len = sizeof(T);
                while (!buffer.push(t, len, offset)) write();
        }

        template <typename T>
        constexpr void fprint_ex(const T& t, size_t len) noexcept
        {
                size_t offset = 0;
                while (!buffer.push(t, len, offset)) write();
        }
};

enum logger_level : uint32_t {
        info,
        debug,
        warning,
        error,
        trace
};

struct ilf_header {
        char     name[4];
        uint32_t table_count;

        ilf_header() : name{'.', 'i', 'l', 'f'}, table_count(0) {}
};

struct ilf_table {
        // size_t next_offset;
        logger_level level;
        uint32_t     str_len;
        uint32_t     str_offset;
        uint32_t     args_size;
        uint32_t     args_offset;
        uint32_t     literals_size;
        uint32_t     literals_offset;
};

struct logger {
        logger(const char* filename) : stream(filename), ilf_htc_offset(4)
        {
                // stream.fprint(header.name);
                stream.fprint_ex(header.name, 4);
                stream.fprint(header.table_count);
        }

        ~logger()
        {
                // flush current buffer
                stream.write();

                // TODO: make this less hacky
                // move file pointer to beginning
                HANDLE handle = stream.handle;
                int    ret = SetFilePointer(handle, 0, NULL, FILE_BEGIN);
                if (ret == INVALID_SET_FILE_POINTER) {
                        int ret = GetLastError();
                        printf("Invalid set file pointer in ~logger(), %i\n", ret);
                        exit(1);
                }

                // print ilf header
                // stream.fprint(header.name);
                stream.fprint_ex(header.name, 4);
                stream.fprint(header.table_count);
        }

        void log(logger_level level, const char* str, ...)
        {
                header.table_count++;

                const char*      c = str;
                uint32_t         args_size = 0;
                uint32_t         literals_size = 0;
                hard_buffer<256> literals_buffer;
                va_list          args;
                va_start(args, str);
                while (*c++) {
                        if (*c != '%') continue;
                        c++;

                        switch (*c) {
                        case 'c':
                        case 'u':
                        case 'i':
                        case 'f':
                                args_size += 8;
                                va_arg(args, int);
                                break;
                        case 's': {
                                // TODO: this looks ugly, fix it
                                args_size += 8;
                                uint64_t* tmp = reinterpret_cast<uint64_t*>(args);
                                // lets include null char
                                const char* literal = va_arg(args, const char*);
                                size_t      len = strlen(literal) + 1;
                                *tmp = len;
                                uint32_t literal_pad = len % 4;
                                literals_size += len + literal_pad;
                                size_t offset = 0;
                                if (!literals_buffer.push(literal, len, offset)) printf("logger::log(...), failed to copy a literal\n");
                                while (literal_pad--) {
                                        size_t offset = 0;
                                        size_t len = 1;
                                        if (!literals_buffer.push('\0', len, offset)) {
                                                printf("logger:log(...), failed to pad a literal\n");
                                                break;
                                                // return;
                                        }
                                }
                                break;
                        }
                        default:
                                printf("logger::log(...), unsupported format specifier '%c'\n", *c);
                        }
                }

                // construct a ilf table
                uint32_t pointer = SetFilePointer(stream.handle, 0, NULL, FILE_CURRENT);
                // because there might be some unflushed data
                pointer += stream.buffer.index;
                // printf("logger::log(..), pointer %u\n", pointer);

                ilf_table table;
                table.level = level;
                table.str_len = static_cast<uint32_t>(c - str);
                uint32_t str_pad = table.str_len % 8;
                table.str_offset = pointer + sizeof(ilf_table);
                table.args_size = args_size;
                table.args_offset = table.str_offset + table.str_len + str_pad;
                table.literals_size = literals_size;
                table.literals_offset = table.args_offset + table.args_size;

                // write a table
                stream.fprint(table.level);
                stream.fprint(table.str_len);
                stream.fprint(table.str_offset);
                stream.fprint(table.args_size);
                stream.fprint(table.args_offset);
                stream.fprint(table.literals_size);
                stream.fprint(table.literals_offset);
                stream.fprint(str);
                stream.fprint('\0');
                while (str_pad--) stream.fprint('\0');
                // this should work, because va_list == char*
                va_start(args, str);
                stream.fprint_ex(reinterpret_cast<const char*>(args), table.args_size);
                stream.fprint_ex(literals_buffer.data, literals_buffer.index);
        }

        streamer stream;
        // ilf header table count offset
        const uint32_t ilf_htc_offset;
        ilf_header     header;
};

struct ilfer {
        HANDLE handle;
        ilfer(const char* filename)
        {
                // open file
                handle = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (handle == INVALID_HANDLE_VALUE) {
                        int ret = GetLastError();
                        printf("Invalid Handle in CreateFile, %i\n", ret);
                        exit(1);
                }
                // start reading it
                SetFilePointer(handle, 0, NULL, FILE_BEGIN);

                ilf_header header;
                ReadFile(handle, &header, sizeof(ilf_header), NULL, NULL);

                char string_buffer[256];
                char args_buffer[256];
                char literals_buffer[256];

                for (uint32_t i = 0; i < header.table_count; i++) {
                        uint32_t pointer = SetFilePointer(handle, 0, NULL, FILE_CURRENT);
                        // printf("ilfer::ilfer(), pointer %u\n", pointer);

                        ilf_table table;
                        ReadFile(handle, &table, sizeof(ilf_table), NULL, NULL);

                        /* printf("ilf_table, level %u, str_len %u, str_offset %u, args_size %u, args_offset %u, literals_size %u, literals_offset %u\n",
                               table.level, table.str_len, table.str_offset, table.args_size, table.args_offset, table.literals_size, table.literals_offset); */

                        if (table.str_len >= 256) continue;
                        if (table.args_size >= 256) continue;
                        if (table.literals_size >= 256) continue;

                        SetFilePointer(handle, table.str_offset, NULL, FILE_BEGIN);
                        ReadFile(handle, string_buffer, table.str_len, NULL, NULL);
                        SetFilePointer(handle, table.args_offset, NULL, FILE_BEGIN);
                        ReadFile(handle, args_buffer, table.args_size, NULL, NULL);
                        SetFilePointer(handle, table.literals_offset, NULL, FILE_BEGIN);
                        ReadFile(handle, literals_buffer, table.literals_size, NULL, NULL);

                        const char* aux = string_buffer;
                        int         arg_count = 0;
                        uint64_t    literals_offset = 0;
                        while (*aux) {
                                // printf("%c ", *aux);
                                if (*aux != '%') {
                                        aux++;
                                        continue;
                                }
                                aux++;

                                switch (*aux) {
                                case 'c':
                                case 'u':
                                case 'i':
                                case 'f':
                                        arg_count++;
                                        break;
                                case 's': {
                                        // UGLYYYY
                                        uint64_t* size = reinterpret_cast<uint64_t*>(args_buffer + arg_count * sizeof(char*));
                                        uint64_t  pad = *size % 4;
                                        char*     pointer = literals_buffer + literals_offset;
                                        memcpy(args_buffer + arg_count * sizeof(char*), &pointer, 8);
                                        literals_offset += *size + pad;
                                        arg_count++;
                                        break;
                                }
                                }
                        }

                        static const char* logger_levels_string[] = {"[info] ", "[debug] ", "[warn] ", "[error] ", "[trace] "};
                        // printf("%s", logger_levels_string[table.level]);
                        // printf("%s\n", string_buffer);

                        printf("%s", logger_levels_string[table.level]);
                        vprintf(string_buffer, reinterpret_cast<va_list>(args_buffer));
                }
        }

        ~ilfer()
        {
                CloseHandle(handle);
        }
};

int main()
{
        // Scopes are cheap hacks to deal with desctructors and file handles
        LARGE_INTEGER F;
        LARGE_INTEGER S;
        LARGE_INTEGER E;
        QueryPerformanceFrequency(&F);

        {
                logger log = {"logs.ilf"};
                QueryPerformanceCounter(&S);
                for (int i = 0; i < 1000000; i++) log.log(logger_level::info, "Hello there %i, %c, %u, %f, %f, %s\n", 7, 'T', 1232435465, 234.98, 12.9f, "SOS");
                QueryPerformanceCounter(&E);
        }

        double delta = static_cast<double>(E.QuadPart - S.QuadPart);
        delta /= F.QuadPart;
        printf("qlog 1 000 000 logs time : %.4f\n", delta);
        /* {
                ilfer ilf = {"logs.ilf"};
        } */
        return 0;
}
