//
// Created by dmerkushov on 15.01.23.
//

#ifndef SOAPSTAB_CLASS_MACROS_H
#define SOAPSTAB_CLASS_MACROS_H

#include <string>

#define IBB_METHOD_NAME methodName(__PRETTY_FUNCTION__)
#define IBB_CLASS_NAME className(__PRETTY_FUNCTION__)

namespace soapstab {

inline std::string_view methodName(const std::string_view &prettyFunction) {
    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;

    return prettyFunction.substr(begin, end);
}

inline std::string_view className(const std::string_view &prettyFunction) {
    size_t colons = prettyFunction.find("::");
    if(colons == std::string::npos)
        return "::";
    size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
    size_t end = colons - begin;

    return prettyFunction.substr(begin, end);
}
} // namespace soapstab

#endif // SOAPSTAB_CLASS_MACROS_H
