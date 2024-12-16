//
// Created by dmerkushov on 05.11.23.
//

#include "metrics_formatter.h"

#include <sstream>

using namespace std;
using namespace soapstab;

shared_ptr<string> prometheus_metrics_formatter::format_metrics(metrics_registry &registry) {
    string prometheus = registry.prometheus_txt();

    // TODO Implement own Prometheus formatter, without registry
    return make_shared<std::string>(prometheus);
}
