//
// Created by dmerkushov on 09.04.23.
//

#ifndef SOAPSTAB_METRIC_H
#define SOAPSTAB_METRIC_H

#include "../logging/logging.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace soapstab {

/**
 * @brief Quantity of buckets in hashtables (std::unordered_map)
 */
const std::size_t hashtable_bucket_count = 16;

/**
 * @brief Timestamp type to use with Prometheus. Seconds since epoch (1 Jan 1970, 0:00:00 GMT)
 */
using timestamp_t = int64_t;

/**
 * @brief The value type to use with Prometheus
 */
using value_t = double;

/**
 * @brief Metric value
 */
struct metric_value_t {
    /**
     * @brief The value itself
     */
    value_t value;

    /**
     * @brief Timestamp of the value
     */
    timestamp_t timestamp;
};

using label_map_t = std::unordered_map<std::string, std::string>;
using label_pair_t = label_map_t::value_type;

/**
 * @brief Hash function for label maps
 */
struct label_values_map_t_hash {
    std::size_t operator()(const label_map_t &labels_map) const;
};

using values_by_labels_t = std::unordered_map<label_map_t, metric_value_t, label_values_map_t_hash>;
using values_by_labels_pair_t = values_by_labels_t::value_type;

class metric {
public:
    metric() = delete;
    metric(metric &) = delete;

    virtual ~metric() = default;

    /**
     * @brief Set the value of the metric with the given label values
     * @param label_map
     * @param value
     * @param timestamp The timestamp which is to set for the value. Defaults to the current time
     */
    virtual void set(label_map_t &label_map, value_t value, timestamp_t timestamp = current_unixtime_sec());

    /**
     * @brief Set the value of the metric, expecting the labels to be the same as the defaults
     * @param value
     */
    void set(value_t value);

    /**
     * @brief Add the given diff to the metric value
     * @param label_map
     * @param diff
     * @param timestamp The timestamp which is to set for the value. Defaults to the current time
     */
    virtual void inc(label_map_t &label_map, value_t diff = 1.0, timestamp_t timestamp = current_unixtime_sec());

    /**
     * @brief Add the given diff to the metric value, expecting the labels to be the same as the defaults. Current time will be set for the metric timestamp
     * @param value
     */
    virtual void inc(value_t diff = 1.0);

    /**
     * @return The name of the metric
     */
    std::string_view name();

    /**
     * @brief Get the value of the metric for the default label values
     * @return The current value of the metric
     */
    value_t value();

    /**
     * @brief Get the value of the metric for the given label values
     * @param label_map
     * @return The current value of the metric, or the initial value of the metric if the supplied label value map had not yet existed
     */
    value_t value(label_map_t &label_map);

    /**
     * @brief Prometheus textual representation of the metric
     * @return
     */
    virtual std::string prometheus_txt() const;

    /**
     * @brief Helper method: get current seconds quantity since Unix epoch
     * @return
     */
    static timestamp_t current_unixtime_sec();

protected:
    /**
     * @brief Create an instance of a metric. Set the value for the given default label map to the given initial value of the metric
     * @param name Name of the metric, must be a non-empty string
     * @param help Help string for the metric
     * @param initial_value Initial value of the metric
     * @param label_default_values Default values of all the possible labels for the metric in the form: label_name -> label_value
     */
    metric(const std::string &name, const std::string &type, const std::string &help, value_t initial_value, const label_map_t &label_default_values = label_map_t(hashtable_bucket_count));

    /**
     * @brief Default labels for the metric
     * @return
     */
    const label_map_t &label_defaults() const;

private:
    /**
     * @brief Mutex to isolate mutating methods execution
     */
    std::mutex _mutex;

    /**
     * @brief Name of the metric
     */
    std::string _name;
    std::string _help;
    std::string _type;
    value_t _initial_value;
    label_map_t _label_defaults;
    values_by_labels_t _values_by_labels = values_by_labels_t(hashtable_bucket_count);
};

/**
 * @brief Defines a Prometheus-export-supporting metric class
 * @tparam T
 */
template<typename T>
concept PrometheusTxtExportMetric = requires(T a) {
                                        { a.prometheus_txt() } -> std::convertible_to<std::string_view>;
                                    };

} // namespace soapstab

namespace fmt {

template<>
struct formatter<soapstab::metric_value_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const soapstab::metric_value_t &metric_value, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "{} {}", metric_value.value, metric_value.timestamp);
    }
};

template<>
struct formatter<soapstab::label_pair_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const soapstab::label_pair_t &label_value, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "{}=\"{}\"", label_value.first, label_value.second);
    }
};

template<>
struct formatter<soapstab::label_map_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const soapstab::label_map_t &label_values, FormatContext &ctx) {
        auto j = fmt::join(label_values.begin(), label_values.end(), ",");
        return fmt::format_to(ctx.out(), "{{{}}}", std::move(j));
    }
};

template<>
struct formatter<soapstab::values_by_labels_pair_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const soapstab::values_by_labels_pair_t &values_by_labels_pair, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "{} {}", values_by_labels_pair.first, values_by_labels_pair.second);
    }
};

template<soapstab::PrometheusTxtExportMetric PTEM>
struct formatter<PTEM> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return std::begin(ctx);
    }

    template<typename FormatContext>
    auto format(const PTEM &metric, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "{}", metric.prometheus_txt());
    }
};

} // namespace fmt

#endif // SOAPSTAB_METRIC_H
