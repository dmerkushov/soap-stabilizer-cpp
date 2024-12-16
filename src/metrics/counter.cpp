//
// Created by dmerkushov on 14.04.23.
//

#include "counter.h"

using namespace std;
using namespace soapstab;

namespace soapstab {
const string counter_metric_type = "counter";
}

void counter::inc(value_t diff) {
    if(diff < 0) {
        log::warn("counter::inc(): attempt to decrement a counter: diff={}", diff);
        return;
    }

    log::trace("+counter::inc(): diff {}", diff);
    metric::inc(diff);
    log::trace("-counter::inc()");
}

void counter::inc(label_map_t &label_map, value_t diff, timestamp_t timestamp) {
    if(diff < 0) {
        log::warn("counter::inc(): attempt to decrement a counter by {}", diff);
        return;
    }

    log::trace("+counter::inc(): diff {}", diff);
    metric::inc(label_map, diff, timestamp);
    log::trace("-counter::inc()");
}

counter::counter(const std::string &name, const std::string &help, const label_map_t &label_default_values) : metric(name, counter_metric_type, help, 0.0, label_default_values) {
}