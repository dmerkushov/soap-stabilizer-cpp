#include "../src/config/config.h"
#include "../src/duty/duty.h"
#include "../src/logging/logging.h"

using namespace std;
using namespace soapstab;

int main() {
    log::set_level(log::level::trace);
    config::instance();

    log::debug("+main()");

    log::info("duty: MM {} TG {}", duty::instance().mm_username(), duty::instance().tg_username());

    string new_mm = "bushminvv";
    string new_tg = "viktor_drobek";
    duty::instance().set(new_mm, new_tg);

    log::info("new duty: MM @{} TG @{}", duty::instance().mm_username(), duty::instance().tg_username());

    log::debug("-main()");
}