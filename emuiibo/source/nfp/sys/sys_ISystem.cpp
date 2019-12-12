#include "nfp/sys/sys_ISystem.hpp"
#include "emu/emu_Emulation.hpp"
#include <atomic>
#include <cstdio>
#include <sys/stat.h>

namespace nfp::sys
{
    ams::Result ISystem::Format(DeviceHandle handle)
    {
        return ams::ResultSuccess();
    }

    ams::Result ISystem::GetAdminInfo(ams::sf::Out<AdminInfo> out_info, DeviceHandle handle)
    {
        return ams::ResultSuccess();
    }

    ams::Result ISystem::GetRegisterInfo2(ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle)
    {
        return this->GetRegisterInfo(out_info, handle);
    }

    ams::Result ISystem::SetRegisterInfo(DeviceHandle handle, RegisterInfo info)
    {
        return ams::ResultSuccess();
    }

    ams::Result ISystem::DeleteRegisterInfo(DeviceHandle handle)
    {
        return ams::ResultSuccess();
    }

    ams::Result ISystem::DeleteApplicationArea(DeviceHandle handle)
    {
        return ams::ResultSuccess();
    }

    ams::Result ISystem::ExistsApplicationArea(ams::sf::Out<u8> out_exists, DeviceHandle handle)
    {
        return ams::ResultSuccess();
    }
}