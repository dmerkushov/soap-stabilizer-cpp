//
// Created by dmerkushov on 16.01.23.
//

#ifndef SOAPSTAB_UID_H
#define SOAPSTAB_UID_H

#include <atomic>

namespace soapstab {

class uid {
public:
    static uint64_t next_uid();

private:
    static std::atomic_uint64_t _uid;
};
} // namespace soapstab

#endif // SOAPSTAB_UID_H
