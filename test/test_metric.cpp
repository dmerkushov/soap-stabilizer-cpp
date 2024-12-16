#include "../src/metrics/metrics_registry.h"

#include <thread>

using namespace std;
using namespace soapstab;

namespace fmt {
template<>
struct formatter<chrono::time_point<chrono::system_clock, chrono::nanoseconds>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const chrono::time_point<chrono::system_clock, chrono::nanoseconds> &tp, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "{}", tp.time_since_epoch().count());
    }
};
} // namespace fmt

namespace soapstab {
void th_inc(shared_ptr<counter> cnt, const int32_t q) {
    for(int i = 0; i < q; i++) {
        cnt->inc();
    }
}
} // namespace soapstab

int main() {
    log::set_level(log::level::info);
    log::info("Started");

    auto gauge1 = metrics_registry::instance().create_gauge("http_requests_total", "The total number of HTTP requests", 1.0, {{"method", "post"}, {"code", "200"}});
    gauge1->set(1027);
    gauge1->inc();

    label_map_t labels = {{"method", "post"}, {"code", "400"}};
    auto counter1 = metrics_registry::instance().create_counter("rpc_duration_seconds", "RPC duration in seconds", {{"method", "post"}, {"code", "200"}});
    counter1->set(labels, 3.0);
    const int32_t inc_count = 10000;
    chrono::time_point<chrono::system_clock, chrono::nanoseconds> n1 = std::chrono::system_clock::now();
    for(int i = 0; i < inc_count; i++) {
        counter1->inc();
    }
    chrono::time_point<chrono::system_clock, chrono::nanoseconds> n2 = std::chrono::system_clock::now();
    log::info("n1: {}, n2: {}", n1, n2);
    auto dur = n2 - n1;
    log::info("For {} counter increments in a single thread, we need: {} ns = {} ms. Type {}", inc_count, dur.count(), dur.count() / 1000000, typeid(dur).name());
    counter1->inc(-0.5);

    auto counter2 = metrics_registry::instance().create_counter("http_duration_seconds_total", "Total HTTP duration", {{"method", "post"}, {"code", "200"}});
    const int32_t t_count = 10;
    thread ts[t_count];
    n1 = std::chrono::system_clock::now();
    for(int i = 0; i < t_count; i++) {
        ts[i] = thread(th_inc, counter2, inc_count / t_count);
    }
    for(int i = 0; i < t_count; i++) {
        ts[i].join();
    }
    n2 = std::chrono::system_clock::now();
    dur = n2 - n1;
    log::info("For {} counter increments in {} threads ({} increments per thread), we need: {} ns = {} ms. Type {}",
              inc_count,
              t_count,
              inc_count / t_count,
              dur.count(),
              dur.count() / 1000000,
              typeid(dur).name());

    log::info("After adding another label map:\n{}", metrics_registry::instance().prometheus_txt());
}