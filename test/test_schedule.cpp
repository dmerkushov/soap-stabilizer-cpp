#include "../src/schedule/schedule.h"
#include "test_common.cpp"

using namespace std;
using namespace soapstab;

int run_test() {
    string tp_str("2023-01-30_21-55-39");
    time_point tp = schedule_element::utc_str_to_time_point(tp_str);
    log::info("{} -> {}", tp_str, *(schedule_element::time_point_to_utc_str(tp)));
    return 0;
}