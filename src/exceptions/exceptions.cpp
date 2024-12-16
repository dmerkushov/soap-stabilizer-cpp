//
// Created by dmerkushov on 05.01.23.
//

#include "exceptions.h"

#include "../../thirdparty/spdlog/fmt/fmt.h"

using namespace std;
using namespace soapstab;

const char *soapstab::exceptionDescription(int32_t ex_id) {
    switch(ex_id) {
    case EX_ALL_OK:
        return "EX_ALL_OK: All OK";
    case EX_NULL_POINTER:
        return "EX_NULL_POINTER: Null pointer";
    case EX_UNKNOWN_CHAT_TYPE:
        return "EX_UNKNOWN_CHAT_TYPE: Unknown chat ctype";
    case EX_INSUFFICIENT_ENV:
        return "EX_INSUFFICIENT_ENV: Insufficient environment";
    case EX_UNSUCCESSFUL_TG_CALL:
        return "EX_UNSUCCESSFUL_TG_CALL: Unsuccessful Telegram API call";
    case EX_UNSUCCESSFUL_MM_CALL:
        return "EX_UNSUCCESSFUL_MM_CALL: Unsuccessful Mattermost API call";
    case EX_COULDNT_SEND:
        return "EX_COULDNT_SEND: Could not send a message";
    case EX_WEBSOCKET_FAIL:
        return "EX_WEBSOCKET_FAIL: WebSocket communication failure";
    case EX_SEND_WRONGTYPE:
        return "EX_SEND_WRONGTYPE: Wrong type of message to send";
    case EX_BACKEND_FAIL:
        return "EX_BACKEND_FAIL: Backend library failure";
    case EX_SCHEDULER_ERROR:
        return "EX_SCHEDULER_ERROR: Scheduler error";
    case EX_CONFIG_ERROR:
        return "EX_CONFIG_ERROR: Configuration error";
    case EX_CONFIG_LISTENER_ERROR:
        return "EX_CONFIG_LISTENER_ERROR: Property listener error after setting a property";
    case EX_BAD_LOG_LEVEL:
        return "EX_BAD_LOG_LEVEL: Bad log level";
    default:
        return "Unknown";
    }
}

ibb_exception::ibb_exception(ex_code_t code) noexcept : _code(code), _description(exceptionDescription(code)) {
    _what = fmt::format("{}{}: {}", _what_start, to_string(code), exceptionDescription(code));
}

ibb_exception::ibb_exception(ex_code_t code, const char *description) noexcept : _code(code), _description(description) {
    _what = fmt::format("{}{}: {}: {}", _what_start, to_string(code), exceptionDescription(code), description);
}

ibb_exception::ibb_exception(ex_code_t code, const string &description) noexcept : _code(code), _description(description) {
    _what = fmt::format("{}{}: {}: {}", _what_start, to_string(code), exceptionDescription(code), description);
}

const string &ibb_exception::description() const noexcept {
    return _description;
}

const char *ibb_exception::what() const noexcept {
    return _what.c_str();
}

ex_code_t ibb_exception::code() const noexcept {
    return _code;
}

ibb_exception &ibb_exception::operator=(ex_code_t i) {
    _code = i;
    _description = exceptionDescription(i);
    return *this;
}