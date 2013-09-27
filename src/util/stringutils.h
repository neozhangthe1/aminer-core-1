#pragma once
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

namespace strutils
{
    using boost::algorithm::join;
    using boost::algorithm::to_lower;
    using boost::algorithm::trim;

    template <typename T>
    std::string vectorToString(const std::vector<T>& list)
    {
        std::vector<std::string> tmp;
        tmp.resize(list.size());
        transform(list.begin(), list.end(), tmp.begin(), static_cast<std::string(&)(T)>(std::to_string));
        return boost::algorithm::join(tmp, ",");
    }

    template <>
    inline std::string vectorToString(const std::vector<std::string>& list) {
        return boost::algorithm::join(list, ",");
    }
}
