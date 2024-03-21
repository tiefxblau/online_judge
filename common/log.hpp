#pragma once

#include <iostream>
#include <string>

#include "util.hpp"

namespace ns_log
{
    using namespace ns_util;

    enum{
        INFO,
        WARNING,
        ERROR,
        FATAL,
        DEBUG
    };

    inline std::ostream& Log(const std::string level, const std::string file, int line)
    {
        std::string msg = "[" + level + "]";
        msg += "[" + file + "]";
        msg += "[" + std::to_string(line) + "]";
        msg += "[" + TimeUtil::GetTimeStamp() + "] ";

        std::cout << msg;
        return std::cout;
    }

    #define Log(level) Log(#level, __FILE__, __LINE__)
} // namespace ns_log

