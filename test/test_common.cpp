#include "../src/config/config.h"
#include "../src/exceptions/exceptions.h"
#include "../src/logging/logging.h"

using namespace std;
using namespace soapstab;

int run_test();

int main() {
    log::set_level(log::level::trace);
    config::instance();

    log::debug("+main()");

    int result;
    try {
        result = run_test();
    } catch(int32_t e) {
        log::error("!main(): {}", exceptionDescription(e));
    }

    if(result == 0) {
        log::debug("-main(): ok");
    } else {
        log::error("-main(): result={}", result);
    }

    return result;
}