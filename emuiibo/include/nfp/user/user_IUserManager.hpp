
#pragma once
#include <switch.h>
#include "nfp/user/user_IUser.hpp"
#include "emu/emu_Status.hpp"
#include "emu/emu_Emulation.hpp"

namespace nfp::user
{
    class IUserManager : public ICommonManager
    {
        NFP_REGISTER_CTOR(ICommonManager)

        NFP_COMMON_MANAGER_CREATE_CMD(IUser)

        NFP_COMMON_MANAGER_BASE
    };
}