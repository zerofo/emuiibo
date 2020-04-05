
#pragma once
#include <emu_Types.hpp>

namespace sys {

    void UpdateVirtualAmiiboCache();
    u32 GetVirtualAmiiboCount();
    std::string GetVirtualAmiibo(u32 idx);

}