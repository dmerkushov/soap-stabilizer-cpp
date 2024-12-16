#include "../logging/logging.h"

#include <string>
#include <vector>

namespace soapstab {

/**
 * @brief Template function used to parse configuration property values from their <code>std::string</code> representations
 * @tparam T
 * @param str
 * @return
 */
template<typename T>
T str_to_value(std::string &str);

template<>
std::string str_to_value<>(std::string &str);

template<>
std::vector<std::string> str_to_value<>(std::string &str);

template<>
uint16_t str_to_value<>(std::string &str);

template<>
int32_t str_to_value<>(std::string &str);

template<>
int64_t str_to_value<>(std::string &str);

template<>
bool str_to_value<>(std::string &str);

template<>
log::level::level_enum str_to_value<>(std::string &str);

/**
 * @brief Template functions used to retrieve <code>std::string</code> representations of configuration property values
 * @tparam T
 * @param value
 * @return
 */
template<typename T>
std::string value_to_str(const T &value) {
    std::string result = std::to_string(value);
    return result;
}

template<>
std::string value_to_str<>(const std::string &value);

template<>
std::string value_to_str<>(const log::level::level_enum &value);

template<>
std::string value_to_str<>(const bool &value);
} // namespace soapstab