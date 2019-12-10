
#pragma once
#include "emu/emu_Types.hpp"

namespace emu
{
    EmulationStatus GetStatus();
    void SetStatus(EmulationStatus NewStatus);

    bool IsStatusOnForever();
    bool IsStatusOnOnce();
    bool IsStatusOn(); // OnOnce or OnForever, any of those
    bool IsStatusOff();
}