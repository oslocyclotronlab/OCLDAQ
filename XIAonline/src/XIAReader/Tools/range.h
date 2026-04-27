//
// Created by Vetle Wegner Ingeberg on 04/04/2023.
//

#ifndef RANGE_H
#define RANGE_H
#include <iterator>
// Range class, similar to the python range. Inspired by example on https://en.cppreference.com/w/cpp/iterator/iterator

template<typename T>
class range_object
{
private:
    T _start;
    const T _stop, _step;

public:
    range_object(const T &start, const T &stop, const T &step = 1) : _start( start ), _stop( stop ), _step( step ){}

    const range_object& begin() const { return *this; }
    const range_object& end() const { return *this; }

    bool operator!=(const range_object&) const
    {
        return _start < _stop;
    }

    void operator++()
    {
        _start += _step;
    }

    auto operator*() const { return _start; }
};

template<typename T>
auto range(const T &start, const T &stop, const T &inc = 1)
-> range_object<T>
{
    return { std::forward<T>(start, stop, inc) };
}
#endif // RANGE_H
