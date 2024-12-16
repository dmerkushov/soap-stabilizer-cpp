//
// Created by dmerkushov on 09.04.23.
//

#include "metrics_registry.h"

#include <mutex>

using namespace std;
using namespace soapstab;

std::shared_ptr<gauge> metrics_registry::create_gauge(const std::string &name, const std::string &help, value_t initial_value, const label_map_t &label_default_values) {
    std::unique_lock _lock(_rw_mutex);
    std::shared_ptr<gauge> g(new gauge(name, help, initial_value, label_default_values));
    _metrics[name] = g;
    return g;
}

std::shared_ptr<counter> metrics_registry::create_counter(const std::string &name, const std::string &help, const label_map_t &label_default_values) {
    std::unique_lock _lock(_rw_mutex);
    std::shared_ptr<counter> c(new counter(name, help, label_default_values));
    _metrics[name] = c;
    return c;
}

namespace fmt {

template<>
struct formatter<metrics_map_pair_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const metrics_map_pair_t &metrics_map_pair, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "{}", *(metrics_map_pair.second));
    }
};

} // namespace fmt

std::string metrics_registry::prometheus_txt() {
    std::shared_lock _lock(_rw_mutex);
    return fmt::format("{}", fmt::join(_metrics.begin(), _metrics.end(), "\n\n"));
}