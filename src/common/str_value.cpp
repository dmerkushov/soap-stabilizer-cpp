//
// Created by dmerkushov on 28.05.23.
//

#include "str_value.h"

#include "../exceptions/exceptions.h"

#include <boost/algorithm/string.hpp>
#include <set>

const std::string STR_TRUE("true");
const std::string STR_FALSE("false");

using namespace std;
using namespace soapstab;

namespace soapstab {
template<>
std::string str_to_value<>(std::string &str) {
    return std::string(str);
}

template<>
std::vector<std::string> str_to_value<>(std::string &str) {
    std::string unsplitted = str;
    std::vector<std::string> result;
    boost::split(result, unsplitted, boost::is_any_of(","));

    return result;
}

template<>
uint16_t str_to_value<>(std::string &str) {
    return (uint16_t) stoi(str);
}

template<>
int32_t str_to_value<>(std::string &str) {
    return stoi(str);
}

template<>
int64_t str_to_value<>(std::string &str) {
    return stoll(str);
}

template<>
bool str_to_value<>(std::string &str) {
    return STR_TRUE == str;
}

template<>
log::level::level_enum str_to_value<>(std::string &str) {
    static const set<string> available_levels = {"off", "critical", "error", "warn", "info", "debug", "trace"};
    if(!available_levels.contains(str)) {
        throw EX_BAD_LOG_LEVEL;
    }
    return log::level::from_str(str);
}

template<>
std::string value_to_str<>(const std::string &value) {
    return std::string(value);
}

template<>
std::string value_to_str<>(const log::level::level_enum &value) {
    return log::level::to_string_view(value).data();
}

template<>
std::string value_to_str<>(const bool &value) {
    return value ? std::string(STR_TRUE) : std::string(STR_FALSE);
}
} // namespace soapstab
