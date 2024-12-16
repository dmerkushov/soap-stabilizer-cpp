//
// Created by dmerkushov on 05.11.23.
//

#ifndef SOAPSTAB_METRICS_FORMATTER_H
#define SOAPSTAB_METRICS_FORMATTER_H

#include "metrics_registry.h"

#include <memory>
#include <string>

namespace soapstab {

class metrics_formatter {
public:
    metrics_formatter() = default;
    ~metrics_formatter() = default;

    /// Format the metrics according to the formatter properties
    virtual std::shared_ptr<std::string> format_metrics(metrics_registry &registry) = 0;
};

class prometheus_metrics_formatter : public metrics_formatter {
public:
    prometheus_metrics_formatter() = default;
    virtual ~prometheus_metrics_formatter() = default;

    std::shared_ptr<std::string> format_metrics(metrics_registry &registry) override;
};

} // namespace intbusbot

#endif // SOAPSTAB_METRICS_FORMATTER_H
