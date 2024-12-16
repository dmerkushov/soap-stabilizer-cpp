//
// Created by dmerkushov on 19.06.23.
//

#include "metrics_server.h"

#include "../config/config.h"
#include "../logging/logging.h"
#include "metrics_registry.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

using namespace std;
using namespace soapstab;

metrics_server::metrics_server() {
    log::debug("metrics_server()");
}

void accept_metrics_requests(boost::asio::ip::tcp::acceptor &acceptor, boost::asio::ip::tcp::socket &socket) {
    acceptor.async_accept(socket, [&](boost::beast::error_code ec) {
        if(ec) {
            log::error("!accept_metrics_requests()::async_accept_lambda: error {}: {}. Will not process the request", ec.value(), ec.message());
        }
        if(!ec) {
            make_shared<metrics_http_connection>(move(socket))->process();
        }
        accept_metrics_requests(acceptor, socket);
    });
}

void metrics_server::start_listening() {
    log::debug("+metrics_server::start_listening()");

    const auto ip_address = boost::asio::ip::make_address(config::instance().get_config_property_value<string>(config::METRICS_SERVER_LISTEN_IPADDRESS));
    const uint16_t port = config::instance().get_config_property_value<uint16_t>(config::METRICS_SERVER_LISTEN_PORT);

    log::info("metrics_server::start_listening(): Metrics server will listen on address {}, port {}. Prometheus endpoint is {}",
              ip_address.to_string(),
              port,
              config::instance().get_config_property_value<string>(config::METRICS_SERVER_PATH_PROMETHEUS));

    boost::asio::ip::tcp::acceptor acceptor {_ioc, {ip_address, port}};
    boost::asio::ip::tcp::socket socket {_ioc};

    accept_metrics_requests(acceptor, socket);

    if(_ioc.stopped()) {
        log::trace("metrics_server::start_listening(): the IO context is stopped, need to restart before running");
        _ioc.restart();
    }

    log::debug("-metrics_server::start_listening(): will now run the IO context in the same thread");
    _ioc.run();
}

void metrics_server::stop_listening() {
    log::debug("+metrics_server::stop_listening()");
    _ioc.stop();
    log::debug("-metrics_server::stop_listening()");
}

metrics_http_connection::metrics_http_connection(boost::asio::ip::tcp::socket socket) : _socket(std::move(socket)) {
    log::debug("metrics_http_connection::ctor(): uid {}", _uid);
}

void metrics_http_connection::process() {
    log::debug("+metrics_http_connection::process(): uid {}", _uid);

    read_and_check_request();
    set_deadline();

    log::debug("-metrics_http_connection::process(): uid {}", _uid);
}

void metrics_http_connection::set_deadline() {
    log::debug("+metrics_http_connection::set_deadline(): uid {}", _uid);

    auto self = shared_from_this();

    _deadline.async_wait([self](boost::beast::error_code ec) {
        log::debug("+metrics_http_connection::set_deadline()::async_lambda: uid {}", self->_uid);
        if(!ec) {
            log::info("metrics_http_connection::set_deadline()::async_lambda: deadline reached for uid {}", self->_uid);
            self->_deadline_reached = true;

            // Close socket to cancel any outstanding operation.
            self->_socket.close(ec);
        }
        log::debug("-metrics_http_connection::set_deadline()::async_lambda: uid {}", self->_uid);
    });

    log::debug("-metrics_http_connection::set_deadline(): uid {}", _uid);
}

void metrics_http_connection::read_and_check_request() {
    log::debug("+metrics_http_connection::read_and_check_request(): uid {}", _uid);
    auto self = shared_from_this();

    boost::beast::http::async_read(_socket, _buffer, _request, [self](boost::beast::error_code ec, size_t bytes_transferred) {
        log::debug("+metrics_http_connection::read_and_check_request()::async_lambda: uid {}", self->_uid);

        if(self->_deadline_reached) {
            log::debug("-metrics_http_connection::read_and_check_request()::async_lambda: uid {}: the call was due to a deadline-reached event", self->_uid);
            return;
        }

        if(log::should_log(log::level::trace)) {
            size_t strdatsize = config::instance().get_config_property_value<int64_t>(config::METRICS_SERVER_BUFFER_SIZE_BYTES);
            char strdat[strdatsize];
            stringstream reqbodyss;
            for(auto it = self->_request.body().cdata().begin(); it != self->_request.body().cdata().end(); it++) {
                strncpy(strdat, (const char *) (*it).data(), (*it).size() > strdatsize ? strdatsize : (*it).size());
                strdat[(*it).size()] = 0;
                reqbodyss << string(strdat);
            }

            log::trace("metrics_http_connection::read_and_check_request()::async_lambda: uid {}: requested: {} {}, bytes in: {}, request body: {}",
                       self->_uid,
                       self->_request.base().method_string().to_string(),
                       self->_request.base().target().to_string(),
                       bytes_transferred,
                       reqbodyss.str());
        }

        if(ec) {
            log::error("!metrics_http_connection::read_and_check_request()::async_lambda: uid {}: error {}: {}. Will not process the request", self->_uid, ec.value(), ec.message());
            self->prepare_response(http::status::internal_server_error, ec.message());
        } else if(self->_request.method() != boost::beast::http::verb::get) {
            log::error("!metrics_http_connection::read_and_check_request()::async_lambda: uid {}: request method is not GET: actually is {}", self->_uid, self->_request.method_string().to_string());
            self->prepare_response(http::status::method_not_allowed);
        } else {
            string expected_endpoint = config::instance().get_config_property_value<string>(config::METRICS_SERVER_PATH_PROMETHEUS);
            string actual_endpoint = self->_request.target().to_string();
            if(actual_endpoint != expected_endpoint) {
                log::error("!metrics_http_connection::read_and_check_request()::async_lambda: uid {}: request target is not {}: actually is {}", self->_uid, expected_endpoint, actual_endpoint);
                self->prepare_response(http::status::not_found);
            } else {
                self->prepare_response();
            }
        }

        self->write_response();

        log::debug("-metrics_http_connection::read_and_check_request()::async_lambda: uid {}", self->_uid);
    });

    log::debug("-metrics_http_connection::read_and_check_request(): uid {}", _uid);
}

void metrics_http_connection::prepare_response(http::status http_status, boost::optional<std::string> msg) {
    log::debug("+metrics_http_connection::prepare_response(): http_status {} {}, msg {}", (unsigned) http_status, http_status, msg.has_value() ? msg.value() : "(no val)");

    _response.result(http_status);
    _response.set(http::field::server, config::instance().get_config_property_value<string>(config::METRICS_SERVER_IDENTIFICATION));
    _response.set(boost::beast::http::field::connection, "close");

    if(http_status != http::status::ok) {
        if(msg.has_value()) {
            beast::ostream(_response.body()) << msg.value();
        } else {
            beast::ostream(_response.body()) << http_status;
        }
    } else {
        _response.set(boost::beast::http::field::content_type, "text/plain");
        boost::beast::ostream(_response.body()) << *(_prometheus.format_metrics(metrics_registry::instance()));
    }

    _response.content_length(_response.body().size());

    log::debug("-metrics_http_connection::prepare_response(): uid {}: content length {}", _uid, _response.body().size());
}

void metrics_http_connection::write_response() {
    log::debug("+metrics_http_connection::write_response(): uid {}", _uid);

    auto self = shared_from_this();
    boost::beast::http::async_write(_socket, _response, [self](boost::beast::error_code ec, size_t) {
        log::debug("+metrics_http_connection::write_response()::async_lambda: uid {}", self->_uid);

        if(ec) {
            log::warn("!metrics_http_connection::write_response()::async_lambda: uid {}: write unsuccessful: error {}: {}", self->_uid, ec.value(), ec.message());
        }

        // self->_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_type::shutdown_both, ec);
        self->_socket.close(ec);
        if(ec) {
            log::warn("metrics_http_connection::write_response()::async_lambda: uid {}: could not gracefully shutdown the socket: error {}: {}", self->_uid, ec.value(), ec.message());
        } else {
            log::trace("metrics_http_connection::write_response()::async_lambda: uid {}: socket shutdown complete, cancelling the deadline", self->_uid);
            self->_deadline.cancel(ec);
            if(ec) {
                log::warn("!metrics_http_connection::write_response()::async_lambda: uid {}: could not cancel the deadline: error {}: {}", self->_uid, ec.value(), ec.message());
            }
        }
    });

    log::debug("-metrics_http_connection::write_response(): uid {}", _uid);
}