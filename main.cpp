#include "qlog.hpp"
#include <cstdio>

int main()
{
        QLOG(QLOG_INFO, "these are integers {} and {}", 6, 3);
        return 0;
}