//
// Created by dmerkushov on 07.01.23.
//

#include "config.h"

#include <cstdlib>

using namespace std;
using namespace soapstab;

const string config::HTTP_LISTEN_PORT = "HTTP_LISTEN_PORT";
const string config::LOG_LEVEL = "LOG_LEVEL";
const string config::HTTP_SERVER_CONCURRENCY_THREADS_QUANTITY = "HTTP_SERVER_CONCURRENCY_THREADS_QUANTITY";
const string config::METRICS_SERVER_BUFFER_SIZE_BYTES = "METRICS_SERVER_BUFFER_SIZE_BYTES";
const string config::METRICS_SERVER_LISTEN_IPADDRESS = "METRICS_SERVER_LISTEN_IPADDRESS";
const string config::METRICS_SERVER_LISTEN_PORT = "METRICS_SERVER_LISTEN_PORT";
const string config::METRICS_SERVER_PATH_PROMETHEUS = "METRICS_SERVER_PATH";
const string config::METRICS_SERVER_IDENTIFICATION = "METRICS_SERVER_IDENTIFICATION";
const string config::METRICS_SERVER_RESPONSE_TIMEOUT_MILLIS = "METRICS_SERVER_RESPONSE_TIMEOUT_MILLIS";

const string config::PROP_TYPE_STR = "string";
const string config::PROP_TYPE_UINT16 = "uint16";
const string config::PROP_TYPE_INT32 = "int32";
const string config::PROP_TYPE_INT64 = "int64";
const string config::PROP_TYPE_BOOL = "boolean";
const string config::PROP_TYPE_LOGLEV = "log_level";

const char *config::get_from_env_c(const char *env_var_name, bool throw_if_no_env, bool hide_value, const char *default_value) {
    log::debug("+get_from_env(): env_var_name: {}, throw_if_no_env: {}, hide_value: {}, default_value: {}",
               env_var_name == nullptr ? "null" : env_var_name,
               throw_if_no_env,
               hide_value,
               default_value == nullptr ? "null" : default_value);
    if(env_var_name == nullptr) {
        log::error("!get_from_env(): Null pointer for the environment variable name");
        throw EX_NULL_POINTER;
    }

    const char *value = ::getenv(env_var_name);
    if(value == nullptr) {
        if(throw_if_no_env) {
            log::error("+get_from_env(): cannot find throwIfNoEnv environment variable {}", env_var_name);
            throw EX_INSUFFICIENT_ENV;
        }
        log::trace("get_from_env(): no environment variable {} found, falling back to the default value", env_var_name);
        value = default_value;
    }
    if(log::should_log(log::level::debug)) {
        if(hide_value) {
            log::debug("-get_from_env(): value: *****(hidden)*****");
        } else {
            log::debug("-get_from_env(): value: {}", value);
        }
    }
    return value;
}

config::config() {
    log::debug("+config::config()");
    unique_lock lk(_mutex);
    log::trace("config::config(): lock acquired");

    add_config_property<uint16_t>(HTTP_LISTEN_PORT, 8080);
    add_config_property<log::level::level_enum>(LOG_LEVEL, log::level::debug, false, true, "Global logging level");
    add_config_property<uint16_t>(HTTP_SERVER_CONCURRENCY_THREADS_QUANTITY, 1);
    add_config_property<string>(METRICS_SERVER_IDENTIFICATION, "IntbusBot Metrics HTTP Server");
    add_config_property<int32_t>(METRICS_SERVER_RESPONSE_TIMEOUT_MILLIS, 5000);
    add_config_property<int64_t>(METRICS_SERVER_BUFFER_SIZE_BYTES, 8192);
    add_config_property<string>(METRICS_SERVER_LISTEN_IPADDRESS, "0.0.0.0");
    add_config_property<uint16_t>(METRICS_SERVER_LISTEN_PORT, 8081);
    add_config_property<string>(METRICS_SERVER_PATH_PROMETHEUS, "/prometheus");

    log::debug("-config::config()");
}

void config::add_property_listener(const property_name &name, config_property_listener &&listener) {
    std::unique_lock lk(_mutex);
    if(!_manageable_properties.contains(name)) {
        log::error("!config::set_config_property_value(): property {} is not manageable or does not exist", name);
        throw EX_CONFIG_ERROR;
    }

    _manageable_properties[name].push_back(listener);
}

std::set<property_name> config::manageable_config_property_names() {
    std::unique_lock lk(_mutex);
    set<property_name> names;

    for(auto it : _manageable_properties) {
        string name = it.first;
        names.insert(name);
    }

    return names;
}

const std::string &config::manageable_config_property_description(property_name &name) {
    std::unique_lock lk(_mutex);

    if(!_property_descriptions.contains(name)) {
        log::error("!config::manageable_config_property_description(): property {} has no description", name);
        throw EX_CONFIG_ERROR;
    }

    return _property_descriptions[name];
}

const std::string config::get_config_property_value_for_output(property_name &name) {
    std::unique_lock lk(_mutex);

    if(!_property_values.contains(name)) {
        log::error("!config::get_config_property_value_var(): no property named {}", name);
        throw EX_CONFIG_ERROR;
    }

    const config_property_value_var &value_var = _property_values[name];
    string result;
    if(value_var.type() == BOOST_CORE_TYPEID(std::string)) {
        result = property_value_for_output<std::string>(name, boost::strict_get<std::string>(value_var));
    } else if(value_var.type() == BOOST_CORE_TYPEID(uint16_t)) {
        result = property_value_for_output<uint16_t>(name, boost::strict_get<uint16_t>(value_var));
    } else if(value_var.type() == BOOST_CORE_TYPEID(int32_t)) {
        result = property_value_for_output<int32_t>(name, boost::strict_get<int32_t>(value_var));
    } else if(value_var.type() == BOOST_CORE_TYPEID(int64_t)) {
        result = property_value_for_output<int64_t>(name, boost::strict_get<int64_t>(value_var));
    } else if(value_var.type() == BOOST_CORE_TYPEID(bool)) {
        result = property_value_for_output<bool>(name, boost::strict_get<bool>(value_var));
    } else if(value_var.type() == BOOST_CORE_TYPEID(log::level::level_enum)) {
        result = property_value_for_output<log::level::level_enum>(name, boost::strict_get<log::level::level_enum>(value_var));
    } else {
        log::error("!config_property_value_var_to_string(): unexpected type: {}", boost::core::demangle(value_var.type().name()));
        throw EX_CONFIG_ERROR;
    }

    return result;
}

const std::string &config::get_config_property_type_for_output(property_name &name) {
    std::unique_lock lk(_mutex);

    if(config_property_type(name) == BOOST_CORE_TYPEID(std::string)) {
        return PROP_TYPE_STR;
    } else if(config_property_type(name) == BOOST_CORE_TYPEID(uint16_t)) {
        return PROP_TYPE_UINT16;
    } else if(config_property_type(name) == BOOST_CORE_TYPEID(int32_t)) {
        return PROP_TYPE_INT32;
    } else if(config_property_type(name) == BOOST_CORE_TYPEID(int64_t)) {
        return PROP_TYPE_INT64;
    } else if(config_property_type(name) == BOOST_CORE_TYPEID(bool)) {
        return PROP_TYPE_BOOL;
    } else if(config_property_type(name) == BOOST_CORE_TYPEID(log::level::level_enum)) {
        return PROP_TYPE_LOGLEV;
    } else {
        log::error("!config_property_value_var_to_string(): unexpected type: {}", boost::core::demangle(config_property_type(name).name()));
        throw EX_CONFIG_ERROR;
    }
}

config_property_listener::config_property_listener(std::function<void()> listener_func, const char *name) : _listener_func(listener_func), _name(name) {
}

void config_property_listener::operator()() {
    _listener_func();
}

const std::string &config_property_listener::name() {
    return _name;
}