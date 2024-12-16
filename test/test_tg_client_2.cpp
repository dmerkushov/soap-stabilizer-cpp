#include "../src/client/telegram/tg_client.h"
#include "../src/config/config.h"
#include "test_common.cpp"

#include <boost/algorithm/string.hpp>
#include <chrono>

using namespace std;
using namespace soapstab;

namespace soapstab {} // namespace soapstab

void tg_listen() {
    log::debug("+tg_listen()");

    tg_client::instance().listen_for_updates();

    log::debug("-tg_listen()");
}

int run_test() {
    log::debug("+run_test()");

    thread tg_listen_thread(tg_listen);

    int64_t sleepfor = 300;

    log::info("run_test(): tg_listen_thread started, sleeping for {} seconds", sleepfor);

    std::this_thread::sleep_for(chrono::seconds(sleepfor));

    log::info("run_test(): sleep ended");

    tg_client::instance().stop_listening();
    tg_listen_thread.join();

    log::debug("-run_test()");
    return 0;
}