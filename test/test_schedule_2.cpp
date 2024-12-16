#include "../src/config/config.h"
#include "../src/exceptions/exceptions.h"
#include "../src/logging/logging.h"
#include "../src/schedule/schedule.h"
#include "../src/tasks/task_queue.h"
#include "test_common.cpp"

#include <boost/algorithm/string.hpp>
#include <random>
#include <regex>
#include <vector>

using namespace std;
using namespace soapstab;

namespace soapstab {
random_device rd;
mt19937 rand_gen(rd());
uniform_int_distribution<int32_t> uni(0, 1);
} // namespace soapstab

void run_schedule() {
    schedule::instance().check_schedule_loop();
}

void print_curr_schedule(const char *anno) {
    log::info("print_curr_schedule {}", anno);
    auto sched = schedule::instance().current_schedule();
    for(auto elem : *sched) {
        log::info("{}: {}", anno, elem);
    }
}

void sch_new_task(schedule_element &to_schedule) {
    try {
        schedule::instance().add(to_schedule);
    } catch(int32_t e) {
        log::warn("add failed: {}", exceptionDescription(e));
    }
}

int run_test() {
    log::debug("+run_test()");

    chrono::time_point tp = chrono::system_clock::now() + 2s;

    schedule_element elem1;
    elem1._time_point = tp;
    elem1._task = make_shared<task>(TASKTYPE_ADMIN, make_shared<nlohmann::json>());
    sch_new_task(elem1);

    print_curr_schedule("before");

    thread sched_thread(run_schedule);

    this_thread::sleep_for(1s);

    schedule_element elem2;
    elem2._time_point = tp;
    elem2._task = make_shared<task>(TASKTYPE_ADMIN, make_shared<nlohmann::json>());
    sch_new_task(elem2);

    print_curr_schedule("middle");

    for(int i = 0; i < 30; i++) {
        schedule_element elem_loop;
        elem_loop._time_point = chrono::system_clock::now() + 1s;
        elem_loop._task = make_shared<task>(TASKTYPE_ADMIN, make_shared<nlohmann::json>());
        sch_new_task(elem_loop);
        this_thread::sleep_for(300ms);
        print_curr_schedule(fmt::format("in loop {}", i).c_str());
        if(1 == uni(rand_gen)) {
            // randomly cancelling the task
            schedule::instance().cancel(elem_loop._uid);
            print_curr_schedule(fmt::format("in loop after cancelling {}", i).c_str());
        }
    }

    // The attempt to add a task in the past must be insuccessful
    sch_new_task(elem2);

    this_thread::sleep_for(1s);

    print_curr_schedule("after loop");

    this_thread::sleep_for(1s);

    schedule::instance().stop_check_schedule_loop();

    sched_thread.join();

    print_curr_schedule("after");

    while(!task_queue::instance().empty()) {
        auto t = task_queue::instance().pop();
        log::info("task queue element: {}", t->description());
    }

    log::debug("-run_test()");
    return 0;
}