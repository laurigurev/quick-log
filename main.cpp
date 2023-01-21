#include "qlog.hpp"
#include <cstdio>
#include <windows.h>

int main()
{
        QLOG(QLOG_INFO, "these are parameters {}, {}, {} and {}", 'a', 6.0f, 3, 7.777);
        return 0;
}