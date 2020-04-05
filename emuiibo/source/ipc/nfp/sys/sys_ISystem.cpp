#include <ipc/nfp/sys/sys_ISystem.hpp>

namespace ipc::nfp::sys {

    ams::Result ISystem::Format(DeviceHandle handle) {
        EMU_LOG_FMT("System - Format!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::GetAdminInfo(ams::sf::Out<AdminInfo> out_info, DeviceHandle handle) {
        EMU_LOG_FMT("System - Get AdminInfo!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::GetRegisterInfo2(ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle) {
        EMU_LOG_FMT("System - GetRegisterInfo2!")
        return this->GetRegisterInfo(out_info, handle);
    }

    ams::Result ISystem::SetRegisterInfo(DeviceHandle handle, const RegisterInfo &info) {
        EMU_LOG_FMT("System - SetRegisterInfo!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::DeleteRegisterInfo(DeviceHandle handle) {
        EMU_LOG_FMT("System - DeleteRegisterInfo!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::DeleteApplicationArea(DeviceHandle handle) {
        EMU_LOG_FMT("System - Delete area!")
        return ams::ResultSuccess();
    }

    ams::Result ISystem::ExistsApplicationArea(ams::sf::Out<u8> out_exists, DeviceHandle handle) {
        EMU_LOG_FMT("System - Exists area?")
        out_exists.SetValue(0);
        return ams::ResultSuccess();
    }

}