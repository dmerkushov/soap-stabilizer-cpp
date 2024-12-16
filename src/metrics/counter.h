//
// Created by dmerkushov on 14.04.23.
//

#ifndef SOAPSTAB_COUNTER_H
#define SOAPSTAB_COUNTER_H

#include "metric.h"

namespace soapstab {

class counter : public metric {
public:
    friend class metrics_registry;

    ~counter() override = default;

    void inc(label_map_t &label_map, value_t diff, timestamp_t timestamp = current_unixtime_sec()) override;

    void inc(value_t diff = 1.0) override;

private:
    counter(const std::string &name, const std::string &help, const label_map_t &label_default_values = label_map_t(hashtable_bucket_count));
};

} // namespace soapstab

#endif // SOAPSTAB_COUNTER_H
