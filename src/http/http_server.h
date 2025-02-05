//
// Created by as on 16.12.24.
//

#ifndef SOAP_STABILIZER_CPP_HTTP_SERVER_H
#define SOAP_STABILIZER_CPP_HTTP_SERVER_H

#include "../config/config.h"
#include "../uid/uid.h"
#include "http_request.h"
#include "http_result.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/optional.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>

namespace soapstab {

class http_request_executor {
    virtual std::shared_ptr<http_result> operator()(std::shared_ptr<http_request>);
};

enum http_security_type {
    /// HTTP security type
    HTTP,

    /// HTTPS is not yet supported
    HTTPS
};

class http_port_configuration {
public:
    /// Security type: HTTP or HTTPS
    http_security_type security_type;
};

class http_server : public std::enable_shared_from_this<http_server> {
public:
    /// Start or resume listening for HTTP requests
    void start_listening();

    /// Stop listening for HTTP requests
    void stop_listening();

    void restart_listening();

    /**
     * Set a TCP port for the HTTP server to listen with an executor to process requests on this port. The old configuration associated with the port (if any) is deleted.
     *
     * This can only be done when the server is not listening
     * @param port the port
     * @param port_configuration Configuration for the port. NB: right now, HTTPS is not supported
     * @param request_executor The HTTP request executor for the port
     */
    void set_port(uint16_t port, http_port_configuration port_configuration, http_request_executor &&request_executor);

    /**
     * Get the HTTP security policy for an HTTP port
     * @param port
     * @return If the port configuration is found, its security policy is returned. If the port configuration is not found, an empty std::optional is returned
     */
    std::optional<http_port_configuration> get_port_configuration(uint16_t port);

    /**
     * Remove a TCP port with its executor from the listening configuration.
     *
     * This can only be done when the server is not listening
     * @param port
     */
    void remove_port(uint16_t port);

    /*
     **************************************************************************
     *
     *   Singleton logic
     *
     **************************************************************************
     */

    http_server(http_server &) = delete;
    void operator=(http_server &) = delete;
    static http_server &instance() {
        static http_server instance;
        return instance;
    }

private:
    std::mutex _mutex;

    /// Create an instance of the HTTP server
    http_server();

    /// Boost.ASIO I/O Context for the HTTP server
    boost::asio::io_context _ioc {config::instance().get_config_property_value<uint16_t>(config::HTTP_SERVER_CONCURRENCY_THREADS_QUANTITY)};

    /// Instances of the HTTP request request_executors per port
    std::map<uint16_t, http_request_executor> request_executors;

    /// Per-port configurations (security policies, etc)
    std::map<uint16_t, http_port_configuration> port_configurations;
};

} // namespace soapstab

namespace fmt {

template<>
struct formatter<soapstab::http_port_configuration> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const soapstab::http_port_configuration &http_port_security_value, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "http_port_configuration[security_type={}]", http_port_security_value.security_type);
    }
};

//
} // namespace fmt

#endif // SOAP_STABILIZER_CPP_HTTP_SERVER_H
