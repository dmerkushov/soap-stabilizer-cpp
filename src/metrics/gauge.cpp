//
// Created by dmerkushov on 09.04.23.
//

#include "gauge.h"

using namespace std;
using namespace soapstab;

namespace soapstab {
const string gauge_metric_type = "gauge";
}

gauge::gauge(const std::string &name, const std::string &help, value_t initial_value, const label_map_t &label_default_values) :
        metric(name, gauge_metric_type, help, initial_value, label_default_values) {
}

void gauge::dec(value_t diff) {
    log::trace("gauge::dec(): diff {}", diff);
    inc(-diff);
}