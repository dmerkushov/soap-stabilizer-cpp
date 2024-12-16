//
// Created by dmerkushov on 16.01.23.
//

#include "uid.h"

#include <atomic>

using namespace soapstab;

std::atomic_uint64_t uid::_uid(0L);

uint64_t uid::next_uid() {
    return _uid.fetch_add(1L);
}
