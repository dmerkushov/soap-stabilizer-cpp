#include "config/config.h"
#include "metrics/metrics_server.h"
#include "run_manager/run_manager.h"

#include <random>

using namespace std;
using namespace soapstab;

namespace soapstab {

void setup_logging() {
    log::debug("+setup_logging()");
    log::level::level_enum log_level = config::instance().get_config_property_value<log::level::level_enum>(config::LOG_LEVEL);
    log::info("setup_logging(): setting log level to the configured: {}", log_level);
    log::set_level(log_level);
    log::trace("setup_logging(): log level set to the configured. Now initializing the log level listener");

    config_property_listener log_level_listener = {[]() {
                                                       log::debug("+log_level_listener()");
                                                       log::level::level_enum current_level = log::get_level();
                                                       log::level::level_enum new_level = config::instance().get_config_property_value<log::level::level_enum>(config::LOG_LEVEL);
                                                       log::set_level(new_level);
                                                       log::warn("-log_level_listener(): switched from level {} to {}", current_level, new_level);
                                                   },
                                                   "log_level_listener"};
    config::instance().add_property_listener(config::LOG_LEVEL, std::move(log_level_listener));
    log::debug("-setup_logging()");
}

void run_metrics_server() {
    log::debug("+run_metrics_server()");
    metrics_server::instance().start_listening();
    log::debug("-run_metrics_server()");
}

void stop_metrics_server() {
    log::debug("+stop_metrics_server()");
    metrics_server::instance().stop_listening();
    log::debug("-stop_metrics_server()");
}

// void test_metrics_server() {
//     auto cnt = metrics_registry::instance().create_counter("my_counter", "My own counter");
//     auto gg = metrics_registry::instance().create_gauge("my_gauge", "My own gauge", 0.0);
//
//     std::random_device rd;                              // obtain a random number from hardware
//     std::mt19937 gen(rd());                             // seed the generator
//     std::uniform_real_distribution<> distr(25.0, 63.0); // define the range
//
//     for(auto i = 0; i < 30; i++) {
//         cnt->inc();
//         gg->set(distr(gen));
//         this_thread::sleep_for(chrono::seconds(1));
//     }
// }

} // namespace soapstab

int main(int argc, char **argv) {
    log::set_level(log::level::trace);
    log::info("+main(): starting, with initial log level=TRACE. Reading the configuration...");

    config::instance();
    setup_logging();

    log::info("main(): starting metrics server thread");
    jthread metrics_server_thread(run_metrics_server);

    // test_metrics_server();

    stop_metrics_server();
    metrics_server_thread.join();

    log::info("-main()");

    return EXIT_SUCCESS;
}