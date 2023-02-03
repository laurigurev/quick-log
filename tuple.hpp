#pragma once

namespace qlog {

template <typename, typename...>
struct tuple;

template <typename T>
struct alignas(8) tuple<T> {
        tuple(const T& t) : data(t) {}
        tuple() {}

        T data;
};

template <typename T, typename... Ts>
struct tuple {
        tuple(const T& t, const Ts&... ts) : data(t), rest(ts...) {}
        tuple() {}

        T            data;
        tuple<Ts...> rest;
};

} // namespace qlog
