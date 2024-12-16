//
// Created by dmerkushov on 22.01.23.
//

#include "../src/client/telegram/tg_client.h"
#include "../src/config/config.h"
#include "../src/exceptions/exceptions.h"
#include "../src/logging/logging.h"

#include <cstdint>

using namespace std;
using namespace nlohmann;
using namespace soapstab;

int main() {
    log::set_level(log::level::trace);
    config::instance();

    log::debug("+main()");

    try {
        string message_text = "@stalhk\\_job test";
        bool need_markdown_filtering = false;
        tg_client::instance().send_to_team_chat(message_text, need_markdown_filtering);

        log::debug("-main()");
    } catch(int32_t ex_id) {
        log::error("!main(): {}", exceptionDescription(ex_id));
    }
}