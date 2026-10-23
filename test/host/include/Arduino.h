// Host (PC) replacement for the Arduino core, used to build and unit test
// the firmware logic without an ESP32 toolchain.
#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#define HEX 16
#define DEC 10
#define OCT 8
#define BIN 2

typedef bool boolean;
typedef uint8_t byte;

// Fake monotonic clock: delay() advances it, tests can set it directly.
inline unsigned long hostMillis = 0;
inline unsigned long millis() { return hostMillis; }
inline void delay(unsigned long ms) { hostMillis += ms; }

template <typename T>
const T &constrain(const T &value, const T &low, const T &high)
{
    return value < low ? low : (high < value ? high : value);
}

class String
{
public:
    String() = default;
    String(const char *s) : _s(s ? s : "") {}
    String(const std::string &s) : _s(s) {}
    String(char c) : _s(1, c) {}

    String(int value, int base = DEC) { _s = format(static_cast<long long>(value), base); }
    String(unsigned int value, int base = DEC) { _s = format(static_cast<long long>(value), base); }
    String(long value, int base = DEC) { _s = format(static_cast<long long>(value), base); }
    String(unsigned long value, int base = DEC) { _s = format(static_cast<long long>(value), base); }

    unsigned int length() const { return static_cast<unsigned int>(_s.size()); }
    bool isEmpty() const { return _s.empty(); }
    const char *c_str() const { return _s.c_str(); }

    char operator[](unsigned int index) const { return _s[index]; }
    char &operator[](unsigned int index) { return _s[index]; }

    String &operator+=(const String &other)
    {
        _s += other._s;
        return *this;
    }

    void replace(const String &from, const String &to)
    {
        if (from.isEmpty())
            return;
        std::string::size_type pos = 0;
        while ((pos = _s.find(from._s, pos)) != std::string::npos)
        {
            _s.replace(pos, from._s.size(), to._s);
            pos += to._s.size();
        }
    }

    void toUpperCase()
    {
        std::transform(_s.begin(), _s.end(), _s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    }

    int toInt() const { return std::atoi(_s.c_str()); }

    friend bool operator==(const String &a, const String &b) { return a._s == b._s; }
    friend bool operator!=(const String &a, const String &b) { return a._s != b._s; }
    friend bool operator<(const String &a, const String &b) { return a._s < b._s; }
    friend bool operator>(const String &a, const String &b) { return a._s > b._s; }
    friend String operator+(const String &a, const String &b) { return String(a._s + b._s); }

private:
    std::string _s;

    static std::string format(long long value, int base)
    {
        if (base == DEC)
            return std::to_string(value);
        char buffer[65];
        bool negative = value < 0;
        unsigned long long magnitude = negative ? 0ULL - static_cast<unsigned long long>(value)
                                                : static_cast<unsigned long long>(value);
        int pos = sizeof(buffer) - 1;
        buffer[pos] = '\0';
        do
        {
            int digit = static_cast<int>(magnitude % base);
            buffer[--pos] = static_cast<char>(digit < 10 ? '0' + digit : 'a' + digit - 10);
            magnitude /= base;
        } while (magnitude > 0);
        if (negative)
            buffer[--pos] = '-';
        return std::string(buffer + pos);
    }
};

class SerialClass
{
public:
    void begin(unsigned long) {}

    size_t print(const String &s) { return std::fputs(s.c_str(), stdout) < 0 ? 0 : s.length(); }
    size_t print(const char *s) { return print(String(s)); }
    size_t print(char c) { return print(String(c)); }
    size_t print(int v, int base = DEC) { return print(String(v, base)); }
    size_t print(unsigned int v, int base = DEC) { return print(String(v, base)); }
    size_t print(long v, int base = DEC) { return print(String(v, base)); }
    size_t print(unsigned long v, int base = DEC) { return print(String(v, base)); }

    size_t println() { return print(String("\n")); }
    size_t println(const String &s) { return print(s) + println(); }
    size_t println(const char *s) { return print(String(s)) + println(); }
    size_t println(char c) { return print(c) + println(); }
    size_t println(int v, int base = DEC) { return print(v, base) + println(); }
    size_t println(unsigned int v, int base = DEC) { return print(v, base) + println(); }
    size_t println(long v, int base = DEC) { return print(v, base) + println(); }
    size_t println(unsigned long v, int base = DEC) { return print(v, base) + println(); }

    template <typename... Args>
    int printf(const char *format, Args... args)
    {
        return std::printf(format, args...);
    }
};

inline SerialClass Serial;
