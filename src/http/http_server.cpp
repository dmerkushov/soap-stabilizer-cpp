//
// Created by as on 16.12.24.
//

#include "http_server.h"

#include "../logging/logging.h"

using namespace std;
using namespace soapstab;

std::shared_ptr<http_result> http_request_executor::operator()(std::shared_ptr<http_request>) {
    log::warn("http_request_executor::operator() - this should never be called and always be overloaded");
    return make_shared<http_result>();
}

std::optional<http_port_configuration> http_server::get_port_configuration(uint16_t port) {
    log::debug("+http_server::get_port_configuration(): port={}", port);

    optional<http_port_configuration> result;
    if(port_configurations.contains(port)) {
        result = port_configurations[port];
    }

    log::debug("-http_server::get_port_configuration(): port={}, result={}", port, result);
}

void http_server::set_port(uint16_t port, http_port_configuration port_configuration, http_request_executor &&request_executor) {
    log::debug("+http_server::set_port(): port={}, port_configuration={}", port, port_configuration);
    log::trace("http_server::set_port(): waiting for mutex...");
    unique_lock<mutex> l(_mutex);
    log::trace("http_server::set_port(): lock acquired, continuing");

    port_configurations[port] = port_configuration;
    request_executors[port] = request_executor;

    log::debug("-http_server::set_port()");
}

void http_server::remove_port(uint16_t port) {
    log::debug("+http_server::remove_port(): port={}", port);
    log::trace("http_server::remove_port(): waiting for mutex...");
    unique_lock<mutex> l(_mutex);
    log::trace("http_server::remove_port(): lock acquired, continuing");

    if(log::should_log(log::level::trace)) {
        auto port_config = get_port_configuration(port);
        log::trace("http_server::remove_port(): for port {}, port configuration was: {}", port, port_config);
    }

    if(port_configurations.contains(port)) {
        port_configurations.erase(port);
    }
    if(request_executors.contains(port)) {
        request_executors.erase(port);
    }

    log::debug("-http_server::remove_port()");
}