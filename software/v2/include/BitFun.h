#pragma once

template <class T>
static inline void setBit(T &x, const int pos)
{
    x |= (static_cast<T>(1) << static_cast<T>(pos));
} // end

template <class T>
static inline void clearBit(T &x, const int pos)
{
    x &= ~(static_cast<T>(1) << static_cast<T>(pos));
} // end

template <class T>
static inline bool checkBit(const T &x, const int pos)
{
    bool check = (x & (static_cast<T>(1) << static_cast<T>(pos))) ? true: false;
    return check;
} // end
