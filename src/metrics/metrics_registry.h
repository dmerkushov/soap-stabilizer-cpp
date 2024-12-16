//
// Created by dmerkushov on 09.04.23.
//

#ifndef SOAPSTAB_METRICS_REGISTRY_H
#define SOAPSTAB_METRICS_REGISTRY_H

#include "counter.h"
#include "gauge.h"
#include "metric.h"

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace soapstab {

using metrics_map_t = std::unordered_map<std::string, std::shared_ptr<metric>>;
using metrics_map_pair_t = metrics_map_t::value_type;

class metrics_registry {
public:
    metrics_registry(const metrics_registry &) = delete;
    void operator=(const metrics_registry &) = delete;
    static metrics_registry &instance() {
        static metrics_registry instance;
        return instance;
    }

    //TODO Remove prometheus_txt() from here, after implementing the Prometheus formatter in metrics_formatter.cpp
    std::string prometheus_txt();

    std::shared_ptr<gauge> create_gauge(const std::string &name, const std::string &help, value_t initial_value, const label_map_t &label_default_values = label_map_t(hashtable_bucket_count));

    std::shared_ptr<counter> create_counter(const std::string &name, const std::string &help, const label_map_t &label_default_values = label_map_t(hashtable_bucket_count));

private:
    metrics_registry() = default;
    std::shared_mutex _rw_mutex;
    metrics_map_t _metrics;
};

} // namespace soapstab

#endif // SOAPSTAB_METRICS_REGISTRY_H
