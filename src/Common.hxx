#pragma once

#include <exception>
#include <string>
#include <iostream>
#include <sstream>

#define DEFAULT_EPSILON 1e-7

namespace trading
{
    class TradingError: public std::runtime_error
    {
    public:
        TradingError(const char* msg): std::runtime_error(msg)
        {
        }

        TradingError(const std::string& msg): std::runtime_error(msg.c_str())
        {
        }
    };

	bool essentiallyEqual(double a, double b, double epsilon = DEFAULT_EPSILON);
}

#define LOG_AND_THROW(x) \
{ \
    std::cerr << x << std::endl; \
    std::ostringstream out; \
    out << x; \
    throw ::trading::TradingError(out.str()); \
}
