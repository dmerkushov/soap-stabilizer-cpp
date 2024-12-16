//
// Created by dmerkushov on 19.06.23.
//

#ifndef SOAPSTAB_METRICS_SERVER_H
#define SOAPSTAB_METRICS_SERVER_H

#include "../config/config.h"
#include "../uid/uid.h"
#include "metrics_formatter.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/optional.hpp>
#include <chrono>
#include <memory>
namespace soapstab {

class metrics_server : public std::enable_shared_from_this<metrics_server> {
public:
    /// (Re)start listening for HTTP requests
    void start_listening();

    /// Stop listening for HTTP requests
    void stop_listening();

    /*
     **************************************************************************
     *
     *   Singleton logic
     *
     **************************************************************************
     */

    metrics_server(metrics_server &) = delete;
    void operator=(metrics_server &) = delete;
    static metrics_server &instance() {
        static metrics_server instance;
        return instance;
    }

private:
    /// Create an instance of the metrics server
    metrics_server();

    /// Boost.ASIO I/O Context for the metrics server
    boost::asio::io_context _ioc {config::instance().get_config_property_value<uint16_t>(config::HTTP_SERVER_CONCURRENCY_THREADS_QUANTITY)};
};

/// Single HTTP connection processing the request for metrics
class metrics_http_connection : public std::enable_shared_from_this<metrics_http_connection> {

public:
    metrics_http_connection(boost::asio::ip::tcp::socket socket);

    /// Start processing the connection
    void process();

private:
    /// Deadline reached for this connection
    std::atomic_bool _deadline_reached = false;

    /// The socket for the currently connected client
    boost::asio::ip::tcp::socket _socket;

    /// UID of the connection
    uint64_t _uid {uid::next_uid()};

    /// The buffer for performing reads
    boost::beast::flat_buffer _buffer {(std::size_t) config::instance().get_config_property_value<int64_t>(config::METRICS_SERVER_BUFFER_SIZE_BYTES)};

    /// The request message
    boost::beast::http::request<boost::beast::http::dynamic_body> _request;

    /// The response message
    boost::beast::http::response<boost::beast::http::dynamic_body> _response;

    /// The timer for putting a deadline on connection processing
    boost::asio::steady_timer _deadline {_socket.get_executor(), std::chrono::milliseconds(config::instance().get_config_property_value<int32_t>(config::METRICS_SERVER_RESPONSE_TIMEOUT_MILLIS))};

    /// Setup a deadline for processing a request
    void set_deadline();

    /// Read and check the HTTP request
    void read_and_check_request();

    /**
     * Prepare the response after checking the request
     * @param http_code HTTP code for the response, 200 OK by default
     * @param msg Message body for the response, empty be default
     */
    void prepare_response(boost::beast::http::status http_status = boost::beast::http::status::ok, boost::optional<std::string> msg = boost::none);

    /// Write the HTTP response
    void write_response();

    /// Metrics formatter
    prometheus_metrics_formatter _prometheus;
};

} // namespace soapstab

namespace fmt {
template<>
struct formatter<boost::beast::http::status> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const boost::beast::http::status &status, FormatContext &ctx) {
        std::stringstream ss;
        ss << status;
        return fmt::format_to(ctx.out(), "{}", ss.str());
    }
};

template<>
struct formatter<boost::optional<std::string>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const boost::optional<std::string> &opt, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "{}", opt.has_value() ? "(none)" : opt.value());
    }
};
} // namespace fmt

#endif // SOAPSTAB_METRICS_SERVER_H
