#include "qlog.hpp"
#include <cstdio>
#include <windows.h>

#define LOOPS 10000000

int main()
{
        // QLOG(QLOG_INFO, "these are parameters {}, {}, {} and {}", 'a', 6.0f, 3, 7.777);
        LARGE_INTEGER F, S, E;
        double delta;
        QueryPerformanceFrequency(&F);
        QueryPerformanceCounter(&S);
        for (u64 i = 0; i < LOOPS; i++) QLOG(QLOG_INFO, "these are parameters {}, {}, {} and {}", 'a', 6.0f, 3, 7.777);
        QueryPerformanceCounter(&E);
        delta = static_cast<double>(E.QuadPart - S.QuadPart);
        delta /= F.QuadPart;
        printf("QLOG(...) for %i loops took %fs\n", LOOPS, delta);
        return 0;
}