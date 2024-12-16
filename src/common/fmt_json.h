//
// Created by dmerkushov on 16.01.23.
//

#ifndef SOAPSTAB_FMT_JSON_H
#define SOAPSTAB_FMT_JSON_H

#include "../../thirdparty/nlohmann/json.hpp"
#include "../../thirdparty/spdlog/spdlog.h"

namespace fmt {

/**
 * @brief fmt library formatter for <code>nlohmann::json</code>
 */
template<>
struct formatter<nlohmann::json> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx);

    template<typename FormatContext>
    auto format(const nlohmann::json &json, FormatContext &ctx);
};

/**
 * @brief Implementation of the <code>parse</code> method for the fmt library formatter for <code>nlohmann::json</code>
 * @tparam ParseContext
 * @param ctx
 * @return
 */
template<typename ParseContext>
constexpr auto formatter<nlohmann::json>::parse(ParseContext &ctx) {
    return std::begin(ctx);
}

/**
 * @brief JSON indent size for fmt library formatting. -1 means there will be no newlines
 */
const int32_t nlohmannJsonIndentSize = -1;
/**
 * @brief JSON indent char for fmt library formatting
 */
const char nlohmannJsonIndentChar = ' ';

/**
 * @brief Implementation of the <code>format</code> method for the fmt library formatter for <code>nlohmann::json</code>
 * @tparam ParseContext
 * @param ctx
 * @return
 */
template<typename FormatContext>
auto formatter<nlohmann::json>::format(const nlohmann::json &json, FormatContext &ctx) {
    return fmt::format_to(ctx.out(), "{}", json.dump(nlohmannJsonIndentSize, nlohmannJsonIndentChar));
}

} // namespace fmt

#endif // SOAPSTAB_FMT_JSON_H
