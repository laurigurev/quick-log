#include <cstdio>
#include <type_traits>
#include <cstring>

#define QLOG_PAD 8

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

struct qlog_table_header {
        const size_t level;
        const size_t str_len;
        const size_t str_offset;
        const size_t args_size;
        const size_t args_offset;
};

template <size_t N>
struct qlog_table_body {
        template <typename L>
        constexpr void qlog_construct_body(const size_t& index, const L& l)
        {
                memcpy(data + index, &l, sizeof(L));
        }

        template <typename T, typename... A>
        constexpr void qlog_construct_body(const size_t& index, const T& t, const A&... a)
        {
                memcpy(data + index, &t, sizeof(t));
                qlog_construct_body(index + 1, a...);
        }

        template <typename... A>
        constexpr qlog_table_body(const A&... a)
        {
                qlog_construct_body(0, a...);
        }

        size_t data[N];
};

struct buffer {
        buffer() : idx(0) {}

        char   raw[256];
        size_t idx;
};

struct file_context {
        file_context() : file_pointer(0) {}

        buffer buf;
        size_t file_pointer;
};

static file_context fc;

template <typename... A>
constexpr void qlog_construct_table(const size_t& lvl, const size_t& slen, const size_t& spad, const char* str, const A... a)
{
        // this is for testing
        fc.buf.idx = 0;

        // get argument list parameters
        constexpr size_t alen = sizeofargs(a...);
        constexpr size_t apad = alen % QLOG_PAD;

        // begin contructing a table
        const size_t table_size = sizeof(qlog_table_header) + slen + spad + alen + apad;

        // construct and push a table header
        const qlog_table_header header = {lvl, slen, fc.file_pointer + table_size, alen, fc.file_pointer + table_size + slen + spad};
        fc.file_pointer += table_size;
        memcpy(fc.buf.raw + fc.buf.idx, &header, sizeof(qlog_table_header));
        fc.buf.idx += sizeof(qlog_table_header);

        // push string
        memcpy(fc.buf.raw + fc.buf.idx, str, slen);
        fc.buf.idx += slen + spad;

        // construct and push a table body
        qlog_table_body<alen> body(a...);
        memcpy(fc.buf.raw + fc.buf.idx, &body, sizeof(body));
        fc.buf.idx += sizeof(body);
}

#define QLOG(lvl, str, ...)                                              \
        {                                                                \
                constexpr size_t slen = sizeof(str);                     \
                constexpr size_t spad = slen % QLOG_PAD;                 \
                qlog_construct_table(lvl, slen, spad, str, __VA_ARGS__); \
        }

#include <windows.h>
#define LOOPS 30000000

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