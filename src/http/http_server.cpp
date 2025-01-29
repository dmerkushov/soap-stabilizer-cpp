//
// Created by as on 16.12.24.
//

#include "http_server.h"

#include "../logging/logging.h"

using namespace std;
using namespace soapstab;

std::optional<http_port_configuration> http_server::get_port_configuration(uint16_t port) {
    log::debug("+http_server::get_port_configuration(): port={}", port);

    optional<http_port_configuration> result;
    if(port_securities.contains(port)) {
        result = port_securities[port];
    }

    log::debug("-http_server::get_port_configuration(): port={}, result={}", port, result);
}