#pragma once

#include "types.hpp"

namespace qlog {

template <u32 N>
struct cstring {
        constexpr cstring(const char (&s)[N]) noexcept
        {
                for (u32 i = 0; i < N; i++) _raw[i] = s[i];
        }

        char operator[](const u32 i)
        {
                return _raw[i];
        }
        char& operator[](const u32 i) const
        {
                return _raw[i];
        }

        template <u32 M>
        constexpr u32 contains(const cstring<M>& s) const
        {
                u32 count = 0;
                for (u32 i = 0; i < N - s.len + 1; i++) {
                        if (_raw[i] != s._raw[0]) continue;
                        for (u32 j = 0; j < s.len - 1; j++) {
                                if (_raw[i + j] != s._raw[j]) {
                                        count--;
                                        break;
                                }
                        }
                        count++;
                }
                return count;
        }

        u32  len = N;
        char _raw[N];
};

} // namespace qlog
