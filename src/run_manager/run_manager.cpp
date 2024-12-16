//
// Created by dmerkushov on 03.02.23.
//

#include "run_manager.h"

#include "../logging/logging.h"

using namespace soapstab;

void run_manager::stop() {
    log::info("run_manager::stop(): called stop");
    _running = false;
}

bool run_manager::running() {
    return _running;
}