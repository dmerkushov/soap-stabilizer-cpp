//
// Created by dmerkushov on 27.05.23.
//

#include "test_common.cpp"

#include <string>
#include <vector>

void log_level_test() {
    log::critical("log_level_test(): critical");
    log::error("log_level_test(): error");
    log::warn("log_level_test(): warn");
    log::info("log_level_test(): info");
    log::debug("log_level_test(): debug");
    log::trace("log_level_test(): trace");
}

int run_test() {
    config::instance().add_config_property<uint16_t>("MY_PROP", 16);

    auto p = config::instance().get_config_property_value<uint16_t>("MY_PROP");
    log::debug("run_test(): property is {}", p);

    config_property_listener listener_for_log_level = {[]() {
                                                           log::level::level_enum new_level = config::instance().get_config_property_value<log::level::level_enum>(config::LOG_LEVEL);
                                                           log::critical("LOG LEVEL PROPERTY LISTENER: current active level is {}, setting log level to {}", log::get_level(), new_level);
                                                           log::set_level(new_level);
                                                           log::critical("LOG LEVEL PROPERTY LISTENER: have set the level to {}", log::get_level());
                                                           throw ibb_exception(EX_CONFIG_LISTENER_ERROR, "My err");
                                                       },
                                                       "run_test_listener"};

    config::instance().add_property_listener(config::LOG_LEVEL, std::move(listener_for_log_level));

    try {
        config::instance().set_config_property_value(config::LOG_LEVEL, log::level::level_enum::debug, true);
    } catch(ibb_exception &e) {
        log::error("!run_test(): exception\n{}", e.description());
    }

    log_level_test();

    auto names = config::instance().manageable_config_property_names();

    for(auto it : names) {
        auto description = config::instance().manageable_config_property_description(it);
        auto current_value = config::instance().get_config_property_value_for_output(it);
        auto value_type = config::instance().get_config_property_type_for_output(it);
        log::debug("{:30} = {:60} : {:10} - {}", it, current_value, value_type, description);
    }

    return 0;
}
