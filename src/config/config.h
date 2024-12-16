//
// Created by dmerkushov on 07.01.23.
//

#ifndef SOAPSTAB_CONFIG_H
#define SOAPSTAB_CONFIG_H

#include "../common/str_value.h"
#include "../exceptions/exceptions.h"
#include "../logging/logging.h"

#include <boost/algorithm/string.hpp>
#include <boost/core/typeinfo.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <cstdint>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>

namespace soapstab {

/**
 * @brief Prefix of the property names as corresponding environment variables
 */
const std::string ENV_VAR_PREFIX = "SOAPSTAB_";

/**
 * @brief An alias for boost::variant to use in configuration property values
 */
using config_property_value_var = boost::variant<std::string, uint16_t, int32_t, int64_t, bool, log::level::level_enum>;

/**
 * @brief An alias for std::string used to use as a property name
 */
using property_name = std::string;

/**
 * @brief Concept of a type suitable for configuration properties
 * @tparam T
 */
template<typename T>
concept ConfigPropertySuitable = requires(config_property_value_var v, T t) { v = t; };

/**
 * @brief An alias for std::function<void()> used to indicate a functor as a property listener
 */
class config_property_listener {
public:
    config_property_listener(std::function<void()> listener_func, const char *name = "autonamed listener");
    void operator()();
    const std::string &name();

private:
    std::function<void()> _listener_func;
    std::string _name;
};

/**
 * @brief Configuration class. A singleton. To get an instance of it, use the instance() static member function.
 */
class config {
public:
    /*
     **************************************************************************
     *
     *   Configuration property types
     *
     **************************************************************************
     */

    static const std::string PROP_TYPE_STR;
    static const std::string PROP_TYPE_UINT16;
    static const std::string PROP_TYPE_INT32;
    static const std::string PROP_TYPE_INT64;
    static const std::string PROP_TYPE_BOOL;
    static const std::string PROP_TYPE_LOGLEV;

    /*
     **************************************************************************
     *
     *   Property names
     *
     **************************************************************************
     */

    /// Property name: listening port
    static const property_name HTTP_LISTEN_PORT;

    /// Property name: spdlog library logging level for the IntbusBot
    static const property_name LOG_LEVEL;

    /// Property name: metrics server: quantity of metrics server threads
    static const property_name METRICS_SERVER_CONCURRENCY_THREADS_QUANTITY;

    /// Property name: metrics server: HTTP identification string
    static const property_name METRICS_SERVER_IDENTIFICATION;

    /// Property name: metrics server: response timeout, in milliseconds
    static const property_name METRICS_SERVER_RESPONSE_TIMEOUT_MILLIS;

    /// Property name: metrics server: buffer size
    static const property_name METRICS_SERVER_BUFFER_SIZE_BYTES;

    /// Property name: metrics server: IP address to bind when listening for HTTP metrics requests
    static const property_name METRICS_SERVER_LISTEN_IPADDRESS;

    /// Property name: metrics server: listening port
    static const property_name METRICS_SERVER_LISTEN_PORT;

    /// Property name: metrics server: Prometheus metrics HTTP path
    static const property_name METRICS_SERVER_PATH_PROMETHEUS;

    /*
     **************************************************************************
     *
     *   Configuration property manipulation routines
     *
     **************************************************************************
     */

    /**
     * @brief Get a configuration property value
     * @tparam P Type of the property. Must be one of:
     * <ul>
     * <li>std::string
     * <li>uint16_t
     * <li>int32_t
     * <li>int64_t
     * <li>bool
     * <li>log::level::level_enum
     * </ul>
     * @param name Name of the property
     * @return
     */
    template<ConfigPropertySuitable P>
    P get_config_property_value(const property_name &name) {
        std::unique_lock lk(_mutex);
        if(!_property_values.contains(name)) {
            log::error("!config::get_config_property_value(): no property found for name {}", name);
            throw EX_CONFIG_ERROR;
        }
        auto value_var = _property_values[name];

        if(value_var.empty()) {
            log::error("!config::get_config_property_value(): empty value found for name {}", name);
            throw EX_CONFIG_ERROR;
        }

        std::string valtype = value_var.type().name();
        if(valtype != BOOST_CORE_TYPEID(P).name()) {
            log::error("!config::get_config_property_value(): the property found for name {} is of type {}, but expected to be of type {}",
                       name,
                       boost::core::demangle(value_var.type().name()),
                       boost::core::demangle(BOOST_CORE_TYPEID(P).name()));
            throw EX_CONFIG_ERROR;
        }

        P value = boost::strict_get<P>(value_var);

        if(log::should_log(log::level::trace)) {
            log::trace("config::get_config_property_value(): {} -> {}", name, property_value_for_output(name, value));
        }
        return value;
    }

    /**
     * @brief Set a configuration property value
     *
     * After setting the value, all the currently existing listeners for the property are called in the same thread, in their addition order. Throwing in the listener will result
     * @tparam P
     * @param name Name of the property
     * @param new_value The new value to set
     * @param rethrow_listener_exceptions if true, will collect the listeners' exceptions, and then rethrow them as a single new ibb_exception with a combined description
     * @throws EX_CONFIG_ERROR in case of error during
     */
    template<ConfigPropertySuitable P>
    void set_config_property_value(const property_name &name, P new_value, bool rethrow_listener_exceptions = false) {
        std::unique_lock lk(_mutex);

        if(!_manageable_properties.contains(name)) {
            log::error("!config::set_config_property_value(): property {} is not manageable or does not exist", name);
            throw EX_CONFIG_ERROR;
        }

        config_property_value_var current_value = _property_values[name];
        std::string current_value_type = current_value.type().name();
        if(current_value_type != BOOST_CORE_TYPEID(P).name()) {
            log::error("!config::get_config_property_value(): the property found for name {} is of type {}, but attempted to be set to a value of type {}",
                       name,
                       current_value_type,
                       BOOST_CORE_TYPEID(P).name());
            throw EX_CONFIG_ERROR;
        }

        log::trace("config::set_config_property_value(): current value: {} -> {}", name, property_value_for_output<P>(name, boost::strict_get<P>(current_value)));
        _property_values[name] = new_value;
        log::trace("config::set_config_property_value(): new value set: {} -> {}", name, property_value_for_output<P>(name, boost::strict_get<P>(_property_values[name])));

        std::string listener_exception_descriptions_initial = log::fmt_lib::format("Setting new property value for config property {} caused exceptions in listeners:", name);
        std::string listener_exception_descriptions = listener_exception_descriptions_initial;
        for(config_property_listener listener : _manageable_properties[name]) {
            log::trace("config::set_config_property_value(): executing listener {} on property {}", listener.name(), name);
            try {
                listener();
                log::trace("config::set_config_property_value(): listener {} on property {} executed successfully", listener.name(), name);
            } catch(ex_code_t ex_code) {
                log::warn("!config::set_config_property_value(): listener {} has thrown an ibb_exception when executing on changing property {}: {}",
                          listener.name(),
                          name,
                          exceptionDescription(ex_code));
                if(rethrow_listener_exceptions) {
                    listener_exception_descriptions += log::fmt_lib::format("\nlistener {}: {}", listener.name(), exceptionDescription(ex_code));
                }
            } catch(ibb_exception &e) {
                log::warn("!config::set_config_property_value(): listener {} has thrown an ibb_exception when executing on changing property {}: {}", listener.name(), name, e.description());
                if(rethrow_listener_exceptions) {
                    listener_exception_descriptions += log::fmt_lib::format("\nlistener {}: {}", listener.name(), e.description());
                }
            } catch(...) {
                log::warn("!config::set_config_property_value(): listener {} has thrown when executing on changing property {}", listener.name(), name);
                if(rethrow_listener_exceptions) {
                    listener_exception_descriptions += log::fmt_lib::format("\nlistener {}: threw an unknown error", listener.name());
                }
            }
        }
        if(rethrow_listener_exceptions && listener_exception_descriptions != listener_exception_descriptions_initial) {
            throw ibb_exception(EX_CONFIG_LISTENER_ERROR, listener_exception_descriptions);
        }
    }

    /**
     * @brief Add a configuration property.
     *
     * When initializing, an attempt is made to read the initial value of the property from a corresponding environment variable
     * @tparam P
     * @param name Property name
     * @param default_value Default property value. If the optional has no value and the corresponding environment variable is absent, an exception will be thrown
     * @param is_hidden True if the property value must be hidden from the logs, false otherwise
     * @param is_manageable True if the property must be manageable (i.e. writable) at runtime
     * @param description Property description (needed for property management)
     * @throws EX_CONFIG_ERROR
     * @throws EX_NULL_POINTER
     * @throws EX_INSUFFICIENT_ENV
     * @see ENV_VAR_PREFIX
     */
    template<ConfigPropertySuitable P>
    void add_config_property(const property_name &name, boost::optional<P> default_value, bool is_hidden = false, bool is_manageable = false, std::string description = "") {
        std::unique_lock lk(_mutex);
        if(_property_values.contains(name)) {
            log::error("!config::add_config_property(): Property {} already exists");
            throw EX_CONFIG_ERROR;
        }

        bool throw_if_no_env = !(default_value.has_value());
        std::string env_var = ENV_VAR_PREFIX + name;
        std::string value_str = get_from_env_c(env_var.c_str(), throw_if_no_env, is_hidden, static_cast<const char *>(throw_if_no_env ? nullptr : value_to_str<P>(default_value.value()).c_str()));

        _property_values[name] = str_to_value<P>(value_str);
        _property_descriptions[name] = description;

        if(is_hidden) {
            _hidden_properties.insert(name);
        }

        if(is_manageable) {
            std::list<config_property_listener> v;
            _manageable_properties[name] = v;
        }
    }

    /**
     * @brief Add a configuration property.
     *
     * When initializing, an attempt is made to read the initial value of the property from a corresponding environment variable
     * @tparam P
     * @param name Property name
     * @param default_value Default property value as a const char *
     * @param is_hidden True if the property value must be hidden from the logs, false otherwise
     * @param is_manageable True if the property must be manageable (i.e. writable) at runtime
     * @param description Property description (needed for property management)
     * @throws EX_CONFIG_ERROR
     * @throws EX_NULL_POINTER
     * @throws EX_INSUFFICIENT_ENV
     * @see ENV_VAR_PREFIX
     */
    template<ConfigPropertySuitable P>
    void add_config_property(const property_name &name, const char *default_value, bool is_hidden = false, bool is_manageable = false, std::string description = "") {
        std::string default_value_str = default_value;
        add_config_property<P>(name, str_to_value<P>(default_value_str), is_hidden, is_manageable, description);
    }

    /**
     * @brief Add a property listener for a given property
     * @param name The property name
     * @param listener The listener
     * @see property_name
     * @see config_property_listener
     */
    void add_property_listener(const property_name &name, config_property_listener &&listener);

    /**
     * @brief Prepare a property value for logging
     * @tparam P
     * @param name
     * @param to_log
     * @return
     */
    template<ConfigPropertySuitable P>
    inline std::string property_value_for_output(const property_name &name, const P &to_log) {
        std::unique_lock lk(_mutex);
        if(_hidden_properties.contains(name)) {
            return "**** HIDDEN ****";
        } else {
            return value_to_str(to_log);
        }
    }

    /**
     * @brief Get manageable configuration properties
     * @return A set of names of the manageable configuration properties
     */
    std::set<property_name> manageable_config_property_names();

    /**
     * @brief Get a description of a manageable configuration property
     * @param name Name of the property
     * @return The description of the given property
     */
    const std::string &manageable_config_property_description(property_name &name);

    /**
     * @brief Get a config property value as a string prepared for output
     * @param name Name of the property
     * @return
     */
    const std::string get_config_property_value_for_output(property_name &name);

    /**
     * @brief Get the config property type as a string
     * @param name
     * @return One of:
     * <ul>
     * <li><code>string</code> for <code>std::string</code>
     * <li><code>uint16</code> for <code>uint16_t</code>
     * <li><code>int32</code> for <code>int32_t</code>
     * <li><code>int64</code> for <code>int64_t</code>
     * <li><code>boolean</code> for <code>bool</code>
     * <li><code>log_level</code> for <code>log::level_enum</code>
     * </ul>
     * @ref std::string
     * @ref log::level_enum
     */
    const std::string &get_config_property_type_for_output(property_name &name);

    inline const boost::typeindex::type_info &config_property_type(property_name &name) {
        std::unique_lock lk(_mutex);

        if(!_property_values.contains(name)) {
            log::error("config_property_type(): no property named {}", name);
            throw EX_CONFIG_ERROR;
        }

        return _property_values[name].type();
    }

    /*
     **************************************************************************
     *
     *   Singleton logic
     *
     **************************************************************************
     */

    config(const config &) = delete;
    void operator=(const config &) = delete;
    static config &instance() {
        static config instance;
        return instance;
    }

private:
    config();

    /**
     * @brief Get a value from an environment variable
     * @param env_var_name Name of the environment variable
     * @param throw_if_no_env Is existence of the environment variable required? If <code>throwIfNoEnv==true</code>, in case of absense of the environment variable an exception will be thrown. If
     * <code>throwIfNoEnv==false</code>, in case of absense of the environment variable, the default value will be returned
     * @param hide_value If <code>hideValue==true</code>, the actual value of the variable won't be shown in the logs
     * @param default_value The default value to use if <code>throwIfNoEnv==false</code>
     * @throws EX_NULL_POINTER if envVar is null
     * @throws EX_INSUFFICIENT_ENV if <code>throwIfNoEnv==true</code> and the required environment variable is absent
     * @return
     */
    const char *get_from_env_c(const char *env_var_name, bool throw_if_no_env, bool hide_value, const char *default_value);

    std::map<property_name, config_property_value_var> _property_values;
    std::map<property_name, std::string> _property_descriptions;

    /**
     * @brief Mapping manageable property names to corresponding linked lists of listeners
     *
     * This map is also used to check if a property is manageable
     */
    std::map<property_name, std::list<config_property_listener>> _manageable_properties;

    /**
     * @brief Properties with values that are hidden from the logs
     */
    std::set<property_name> _hidden_properties;

    std::recursive_mutex _mutex;
};

} // namespace soapstab

#endif // SOAPSTAB_CONFIG_H
