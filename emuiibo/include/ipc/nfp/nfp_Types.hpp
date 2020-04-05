
#pragma once
#include <emu_Results.hpp>

namespace ipc::nfp {

    struct DeviceHandle {
        u32 npad_id;
        u8 reserved[4];
    };

    static_assert(sizeof(DeviceHandle) == sizeof(u64), "Invalid DeviceHandle struct");

}