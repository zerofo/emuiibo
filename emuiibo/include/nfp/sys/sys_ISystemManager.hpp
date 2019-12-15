
#pragma once
#include <switch.h>
#include "nfp/sys/sys_ISystem.hpp"
#include "emu/emu_Status.hpp"
#include "emu/emu_Emulation.hpp"

namespace nfp::sys
{
    class ISystemManager : public ICommonManager
    {
        NFP_REGISTER_CTOR(ICommonManager)

        NFP_COMMON_MANAGER_CREATE_CMD(ISystem)

        NFP_COMMON_MANAGER_BASE
    };
}