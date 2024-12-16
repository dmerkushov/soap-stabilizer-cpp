//
// Created by dmerkushov on 23.12.22.
//

#ifndef SOAPSTAB_INTBUS_H
#define SOAPSTAB_INTBUS_H

#include <cstdint>
#include <exception>
#include <string>

namespace soapstab {

using ex_code_t = int32_t;

class ibb_exception final : public std::exception {
public:
    ibb_exception() = delete;
    explicit ibb_exception(ex_code_t code) noexcept;
    ibb_exception(ex_code_t code, const char *description) noexcept;
    ibb_exception(ex_code_t code, const std::string &description) noexcept;
    ibb_exception(ibb_exception &) = default;
    ibb_exception(ibb_exception &&) = default;
    ibb_exception &operator=(const ibb_exception &) = default;
    ibb_exception &operator=(ibb_exception &&) = default;
    ibb_exception &operator=(ex_code_t i);

    ex_code_t code() const noexcept;
    const std::string &description() const noexcept;
    const char *what() const noexcept override;

private:
    ex_code_t _code;
    std::string _description;
    std::string _what;
    std::string _what_start = "ibb_exception ";
};

///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////  Exception codes  ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Exception code: Everything is OK! A dummy exception code
 */
const ex_code_t EX_ALL_OK = 0x0;

////////////////////////////////////  Generic exception codes  ////////////////////////////////////

/**
 * @brief Exception code: Null pointer
 */
const ex_code_t EX_NULL_POINTER = 0x01;

/**
 * @brief Exception code: Cannot find a required environment variable
 */
const ex_code_t EX_INSUFFICIENT_ENV = 0x02;

////////////////////////////////////  Chatting exception codes  ////////////////////////////////////

/**
 * @brief Exception code: Unrecognized chat ctype. See also BaseChat::chatFactory()
 */
const ex_code_t EX_UNKNOWN_CHAT_TYPE = 0x11;

/**
 * @brief Exception code: Unsuccessful Telegram API call
 */
const ex_code_t EX_UNSUCCESSFUL_TG_CALL = 0x12;

/**
 * @brief Exception code: Unsuccessful Mattermost API call
 */
const ex_code_t EX_UNSUCCESSFUL_MM_CALL = 0x13;

/**
 * @brief Exception code: Could not send a message
 */
const ex_code_t EX_COULDNT_SEND = 0x14;

/**
 * @brief WebSocket communication failure
 */
const ex_code_t EX_WEBSOCKET_FAIL = 0x15;

/**
 * @brief Wrong type of message to send
 */
const ex_code_t EX_SEND_WRONGTYPE = 0x16;

/**
 * @brief Backend library failure
 */
const ex_code_t EX_BACKEND_FAIL = 0x17;

/**
 * @brief Scheduler error
 */
const ex_code_t EX_SCHEDULER_ERROR = 0x18;

/**
 * @brief Configuration error
 */
const ex_code_t EX_CONFIG_ERROR = 0x19;

/**
 * @brief Property listener error when setting a property
 */
const ex_code_t EX_CONFIG_LISTENER_ERROR = 0x20;

/**
 * @brief Bad log level
 */
const ex_code_t EX_BAD_LOG_LEVEL = 0x21;

/**
 * @param ex_id Exception id
 * @return Get a description of an exception
 */
const char *exceptionDescription(int32_t ex_id);

} // namespace soapstab

///////////////////////////////////////////////////////////////////////////////////////////
////////////////////  FMT library formatting for third-party entities  ////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#endif // SOAPSTAB_INTBUS_H
