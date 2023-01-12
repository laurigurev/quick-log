#include <cassert>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <windows.h>

#define QLOG_PAD 8

namespace qlog {

template <typename L>
constexpr size_t sizeofargs(const L& l)
{
        static_assert(std::is_integral<L>::value || std::is_floating_point<L>::value);
        return 1;
}

template <typename T, typename... A>
constexpr size_t sizeofargs(const T& t, const A&... a)
{
        static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value);
        return 1 + sizeofargs(a...);
}

// TODO: add a header

struct table_header {
        const size_t table_size;
        const size_t level;
        const size_t str_len;
        const size_t str_offset;
        const size_t args_size;
        const size_t args_offset;
};

struct context {
        context() : idx(0), file_pointer(0)
        {
                memory = reinterpret_cast<char*>(VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE));
                assert(memory != NULL);
                storage = reinterpret_cast<char*>(VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE));
                assert(storage != NULL);

                handle = CreateFile("logs.qlb", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING, NULL);
                assert(handle != INVALID_HANDLE_VALUE);
        }

        ~context()
        {
                flush();
                // TODO: truncate the file
                CloseHandle(handle);
        }

        void flush()
        {
                WriteFile(handle, memory, size, NULL, NULL);
                idx = 0;
        }

        template <typename T>
        constexpr void push_table_body(char* dst, const size_t offset, const T t)
        {
                if (sizeof(T) == 8) {
                        memcpy(dst + offset, &t, 8);
                }
                else if (sizeof(T) == 4) {
                        memcpy(dst + offset, &t, 4);
                        memset(dst + offset + 4, 0, 4);
                }
                else if (sizeof(T) == 2) {
                        memcpy(dst + offset, &t, 2);
                        memset(dst + offset + 2, 0, 6);
                }
                else if (sizeof(T) == 1) {
                        memset(dst + offset, 0, 8);
                        memcpy(dst + offset, &t, 1);
                }
        }

        template <typename T, typename... A>
        constexpr void push_table_body(char* dst, const size_t offset, const T t, const A... a)
        {
                if (sizeof(T) == 8) {
                        memcpy(dst + offset, &t, 8);
                }
                else if (sizeof(T) == 4) {
                        memset(dst + offset, 0, 8);
                        memcpy(dst + offset, &t, 4);
                }
                else if (sizeof(T) == 2) {
                        memset(dst + offset, 0, 8);
                        memcpy(dst + offset, &t, 2);
                }
                else if (sizeof(T) == 1) {
                        memset(dst + offset, 0, 8);
                        memcpy(dst + offset, &t, 1);
                }
                push_table_body(dst, offset + sizeof(size_t), a...);
        }

        template <typename... A>
        constexpr void push_table(const table_header header, const size_t slen, const size_t spad, const char* str, const size_t alen, const size_t apad,
                                  const A... a)
        {
                if (idx + header.table_size < size) {
                        // proceed
                        memcpy(memory + idx, &header, sizeof(table_header));
                        idx += sizeof(table_header);
                        memcpy(memory + idx, str, slen);
                        idx += slen + spad;
                        push_table_body(memory, idx, a...);
                        idx += alen + apad;

                        // printf("qlog::context::push_table(...), idx %llu, file_pointer %llu\n", idx, file_pointer);
                }
                else {
                        push_table_slow(header, slen, spad, str, alen, apad, a...);

                        // printf("qlog::context::push_table_slow(...), idx %llu, file_pointer %llu\n", idx, file_pointer);
                }
                file_pointer += header.table_size;
        }

        template <typename... A>
        constexpr void push_table_slow(const table_header header, const size_t slen, const size_t spad, const char* str, const size_t alen, const size_t apad,
                                       const A... a)
        {
                // create table on secondary buffer
                size_t len = 0;
                memcpy(storage + len, &header, sizeof(table_header));
                len += sizeof(table_header);
                memcpy(storage + len, str, slen);
                len += slen + spad;
                push_table_body(storage, len, a...);
                len += alen + apad;

                // copy as much as we can to fill the buffer,
                // then flush and finally add what we are missing
                const size_t delta = size - idx;
                memcpy(memory + idx, storage, delta);
                flush();
                memcpy(memory + idx, storage + delta, len - delta);

                idx += len - delta;
        }

        static constexpr size_t size = 1024;
        char*                   memory;
        char*                   storage;
        size_t                  idx;
        size_t                  file_pointer;
        HANDLE                  handle;
};

static context ctx;

template <typename... A>
constexpr void construct_table(const size_t lvl, const size_t slen, const size_t spad, const char* str, const A... a)
{
        // get argument list parameters
        constexpr size_t acount = sizeofargs(a...);
        constexpr size_t alen = acount * sizeof(size_t);
        constexpr size_t apad = alen % QLOG_PAD;

        // begin contructing a table
        const size_t table_size = sizeof(table_header) + slen + spad + alen + apad;
        // static_assert(table_size < ctx.size);

        // construct and push a table header
        const table_header header = {
            table_size, lvl, slen, ctx.file_pointer + sizeof(table_header), alen, ctx.file_pointer + sizeof(table_header) + slen + spad};
        ctx.push_table(header, slen, spad, str, alen, apad, a...);
}

} // namespace qlog

#define QLOG(lvl, str, ...)                                               \
        {                                                                 \
                constexpr size_t slen = sizeof(str);                      \
                constexpr size_t spad = slen % QLOG_PAD;                  \
                qlog::construct_table(lvl, slen, spad, str, __VA_ARGS__); \
        }

// #define LOOPS 30000000
#define LOOPS 100000

int main()
{
        printf("Hello, meta-redo!\n");

        LARGE_INTEGER f, s, e;
        double        delta = 0.0;
        QueryPerformanceFrequency(&f);
        QueryPerformanceCounter(&s);
        for (int i = 0; i < LOOPS; i++) {
                QLOG(2, "kjskjdksjd % kjsd%kj\n", false, 5, delta, 99.999f, true);
        }
        QueryPerformanceCounter(&e);

        delta = static_cast<double>(e.QuadPart - s.QuadPart);
        delta /= f.QuadPart;

        printf("qlog(...) for %i loops took %.4fs\n", LOOPS, delta);

        return 0;
}