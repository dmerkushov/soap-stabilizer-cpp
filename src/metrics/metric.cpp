//
// Created by dmerkushov on 09.04.23.
//

#include "metric.h"

#include "../logging/logging.h"

#include <chrono>

using namespace std;
using namespace soapstab;

std::size_t label_values_map_t_hash::operator()(const label_map_t &labels_map) const {
    size_t hash = 0;
    for(const auto &[label, value] : labels_map) {
        hash = hash * 31 + (std::hash<string>()(label) ^ std::hash<string>()(value));
    }
    return hash;
}

metric::metric(const string &name, const string &type, const std::string &help, double initial_value, const label_map_t &label_default_values) :
        _name(name), _help(help), _type(type), _label_defaults(label_default_values) {
    log::trace("metric::metric(): name {}", name);
    _values_by_labels[_label_defaults] = {initial_value, current_unixtime_sec()};
}

timestamp_t metric::current_unixtime_sec() {
    const auto now = chrono::system_clock::now();
    uint64_t now_epoch_sec = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();
    log::trace("metric::current_unixtime_sec(): {}", now_epoch_sec);
    return now_epoch_sec;
}

void metric::set(double value) {
    log::trace("metric::set(): value {}", value);
    set(_label_defaults, value, current_unixtime_sec());
}

void metric::set(label_map_t &label_map, value_t value, timestamp_t timestamp) {
    log::trace("+metric::set(): for label values {}, will set value {} with timestamp {}", label_map, value, timestamp);
    unique_lock l(_mutex);
    log::trace("metric::set(): lock acquired");

    _values_by_labels[label_map] = {value, timestamp};
    log::trace("-metric::set(): {}{} {}", _name, label_map, _values_by_labels[label_map]);
}

std::string metric::prometheus_txt() const {
    string joiner = fmt::format("\n{}", _name);
    string result = fmt::format(
            "# HELP {0} {1}\n"
            "# TYPE {0} {2}\n"
            "{0}{3}",
            _name,
            _help,
            _type,
            fmt::join(_values_by_labels, joiner));
    return result;
}

value_t metric::value(label_map_t &label_map) {
    log::trace("+metric::value(): for label map {}", label_map);
    unique_lock l(_mutex);
    log::trace("metric::value(): lock acquired");

    value_t val;
    auto val_find = _values_by_labels.find(label_map);
    if(val_find != _values_by_labels.end()) {
        val = val_find->second.value;
    } else {
        val = _initial_value;
    }

    log::trace("-metric::value(): {}", val);
    return val;
}

value_t metric::value() {
    log::trace("metric::value()");
    return value(_label_defaults);
}

std::string_view metric::name() {
    log::trace("metric::name()");
    return _name;
}

void metric::inc(label_map_t &label_map, value_t diff, timestamp_t timestamp) {
    log::trace("+metric::inc(): label_map {}, diff {}, timestamp {}", label_map, diff, timestamp);
    unique_lock l(_mutex);
    log::trace("metric::inc(): lock acquired");

    metric_value_t &metric_value = _values_by_labels[label_map];
    metric_value.value += diff;
    metric_value.timestamp = timestamp;

    log::trace("-metric::inc()");
}

void metric::inc(value_t diff) {
    log::trace("metric::inc(): diff {}", diff);
    inc(_label_defaults, diff, current_unixtime_sec());
}

const label_map_t &metric::label_defaults() const {
    log::trace("metric::label_defaults()");
    return _label_defaults;
}