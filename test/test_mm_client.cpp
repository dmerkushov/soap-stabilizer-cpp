#include "../src/client/mattermost/mm_client.h"
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
        shared_ptr<json> post_json = make_shared<json>();
        (*post_json)["channel_id"] = config::instance().mm_admin_channel_id();
        (*post_json)["message"] = "@merkushovda tst";
        mm_client::instance().send_async(post_json);

        log::debug("-main()");
    } catch(int32_t ex_id) {
        log::error("!main(): {}", exceptionDescription(ex_id));
    }
}