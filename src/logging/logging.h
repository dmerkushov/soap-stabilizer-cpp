//
// Created by dmerkushov on 16.01.23.
//

#ifndef SOAPSTAB_LOGGING_H
#define SOAPSTAB_LOGGING_H

#include "../../thirdparty/spdlog/spdlog.h"

#include <optional>

namespace soapstab {
namespace log = ::spdlog;
}

namespace fmt {
template<>
struct formatter<spdlog::level::level_enum> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const spdlog::level::level_enum &level, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "{}", to_string_view(level));
    }
};

template<typename T>
struct formatter<std::optional<T>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const std::optional<T> &opt, FormatContext &ctx) {
        if(opt.has_value()) {
            return fmt::format_to(ctx.out(), "optional[{}]", opt.value());
        } else {
            return fmt::format_to(ctx.out(), "optional[empty]");
        }
    }
};
} // namespace fmt

#endif // SOAPSTAB_LOGGING_H
