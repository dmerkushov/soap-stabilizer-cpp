//
// Created by dmerkushov on 09.04.23.
//

#ifndef SOAPSTAB_GAUGE_H
#define SOAPSTAB_GAUGE_H

#include "metric.h"

#include <atomic>

namespace soapstab {

class gauge : public metric {
public:
    friend class metrics_registry;

    ~gauge() override = default;

    /**
     * @brief Decrement the gauge by the supplied value
     * @param diff
     */
    void dec(value_t diff = 1.0);

private:
    gauge(const std::string &name, const std::string &help, value_t initial_value, const label_map_t &label_default_values = label_map_t(hashtable_bucket_count));
};

} // namespace soapstab

#endif // SOAPSTAB_GAUGE_H
