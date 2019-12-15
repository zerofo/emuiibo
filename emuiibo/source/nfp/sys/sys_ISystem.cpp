#include "nfp/sys/sys_ISystem.hpp"
#include "emu/emu_Emulation.hpp"
#include <atomic>
#include <cstdio>
#include <sys/stat.h>

namespace nfp::sys
{
    ams::Result ISystem::Format(DeviceHandle handle)
    {
        LOG_FMT("System - Format!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::GetAdminInfo(ams::sf::Out<AdminInfo> out_info, DeviceHandle handle)
    {
        LOG_FMT("System - Get AdminInfo!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::GetRegisterInfo2(ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle)
    {
        LOG_FMT("System - GetRegisterInfo - 2!")
        return this->GetRegisterInfo(out_info, handle);
    }

    ams::Result ISystem::SetRegisterInfo(DeviceHandle handle, const RegisterInfo &info)
    {
        LOG_FMT("System - SetRegisterInfo!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::DeleteRegisterInfo(DeviceHandle handle)
    {
        LOG_FMT("System - DeleteRegisterInfo!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::DeleteApplicationArea(DeviceHandle handle)
    {
        LOG_FMT("System - Delete area!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::ExistsApplicationArea(ams::sf::Out<u8> out_exists, DeviceHandle handle)
    {
        LOG_FMT("System - Exists area?")
        return ams::ResultSuccess();
    }
}