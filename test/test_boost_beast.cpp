//
// Created by dmerkushov on 12.03.23.
//
#include "../src/client/telegram/tg_client.h"
#include "../src/config/config.h"
#include "../src/exceptions/exceptions.h"
#include "../src/logging/logging.h"
#include "../src/tasks/task_queue.h"
#include "test_common.cpp"

#include <boost/algorithm/string.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <chrono>

using namespace std;
using namespace soapstab;
namespace bstb = boost::beast;
namespace http = boost::beast::http;

namespace soapstab {} // namespace soapstab

void tg_listen() {
    log::debug("+tg_listen()");

    tg_client::instance().listen_for_updates();

    log::debug("-tg_listen()");
}

int run_test() {
    log::debug("+run_test()");

    bstb::ssl_stream<bstb::tcp_stream> stream(_ioc, _sslCtx);
    bstb::flat_buffer buffer;
    promise<bool> read_promise;
    http::response<http::string_body> res;
    int32_t read_timeout = config::instance().http_request_timeout_sec();
    log::trace("base_client::perform_http_request(): request_uid {}: reading the response with timeout: {} seconds", request_uid, read_timeout);
    http::async_read(stream, buffer, res, [&read_promise, request_uid](bstb::error_code ec, size_t read_bytes) {
        log::debug("+base_client::perform_http_request()::read_result_lambda: request_uid {}", request_uid);
        if(ec) {
            log::error("!base_client::perform_http_request()::read_result_lambda: request_uid {}: Couldn't read the response: {}", request_uid, ec.message());
            read_promise.set_value(false);
            return;
        }
        log::debug("-base_client::perform_http_request()::read_result_lambda: request_uid {}: successfully read {} bytes", request_uid, read_bytes);
        read_promise.set_value(true);
    });
    auto read_future = read_promise.get_future();
    auto read_future_status = read_future.wait_for(chrono::seconds(read_timeout));
    if(read_future_status == future_status::timeout) {
        log::error("!base_client::perform_http_request(): request_uid {}: Timed out when reading the response", request_uid);
        throw EX_BACKEND_FAIL;
    }
    bool read_result = read_future.get();
    if(!read_result) {
        log::error("!base_client::perform_http_request(): request_uid {}: Couldn't read the response", request_uid);
        throw EX_BACKEND_FAIL;
    }

    log::debug("-run_test()");
    return 0;
}