#include "nfp/user/user_IUser.hpp"
#include "emu/emu_Emulation.hpp"
#include <atomic>
#include <cstdio>
#include <sys/stat.h>

namespace nfp::user
{
    IUser::IUser(Service *fwd)
    {
        this->currentAreaAppId = 0;
        this->eventActivate.InitializeAsInterProcessEvent();
        this->eventDeactivate.InitializeAsInterProcessEvent();
        this->eventAvailabilityChange.InitializeAsInterProcessEvent();
        this->state = NfpState_NonInitialized;
        this->deviceState = NfpDeviceState_Unavailable;
        this->fwd_srv = fwd;
    }

    IUser::~IUser()
    {
        serviceClose(this->fwd_srv);
        delete this->fwd_srv;
    }

    ams::Result IUser::Initialize(const ams::sf::ClientAppletResourceUserId &client_aruid, const ams::sf::ClientProcessId &client_pid, const ams::sf::InBuffer &mcu_data)
    {
        LOG_FMT("Process ID: 0x" << std::hex << client_pid.GetValue().value)
        LOG_FMT("ARUID: 0x" << std:: hex << client_aruid.GetValue().value)
        this->state = NfpState_Initialized;
        this->deviceState = NfpDeviceState_Initialized;
        return ams::ResultSuccess();
    }

    ams::Result IUser::Finalize()
    {
        this->state = NfpState_NonInitialized;
        this->deviceState = NfpDeviceState_Finalized;
        return ams::ResultSuccess();
    }

    ams::Result IUser::ListDevices(const ams::sf::OutPointerArray<DeviceHandle> &out_devices, ams::sf::Out<s32> out_count)
    {
        u64 id = 0x20;
        hidScanInput();
        if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) id = (u64)CONTROLLER_PLAYER_1;
        DeviceHandle handle = {};
        handle.npad_id = (u32)id;
        out_devices[0] = handle;
        out_count.SetValue(1);
        return ams::ResultSuccess();
    }

    ams::Result IUser::StartDetection(DeviceHandle handle)
    {
        this->eventActivate.Signal();
        this->eventAvailabilityChange.Signal();
        this->deviceState = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    ams::Result IUser::StopDetection(DeviceHandle handle)
    {
        switch(deviceState)
        {
            case NfpDeviceState_TagFound:
            case NfpDeviceState_TagMounted:
                this->eventDeactivate.Signal();
            case NfpDeviceState_SearchingForTag:
            case NfpDeviceState_TagRemoved:
                this->deviceState = NfpDeviceState_Initialized;
                break;
            default:
                break;
        }
        return ams::ResultSuccess();
    }

    ams::Result IUser::Mount(DeviceHandle handle, u32 type, u32 target)
    {
        this->eventActivate.Signal();
        this->deviceState = NfpDeviceState_TagMounted;
        return ams::ResultSuccess();
    }

    ams::Result IUser::Unmount(DeviceHandle handle)
    {
        this->eventDeactivate.Signal();
        this->deviceState = NfpDeviceState_SearchingForTag;
        return ams::ResultSuccess();
    }

    ams::Result IUser::OpenApplicationArea(ams::sf::Out<u32> npad_id, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(amiibo.ExistsArea(id))
        {
            this->currentAreaAppId = id;
            npad_id.SetValue(handle.npad_id);
            return ams::ResultSuccess();
        }
        return result::ResultAreaNotFound;
    }

    ams::Result IUser::GetApplicationArea(const ams::sf::OutBuffer &data, ams::sf::Out<u32> data_size, DeviceHandle handle)
    {
        if(currentAreaAppId == 0) return result::ResultAreaNotFound;
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        u64 sz = (u64)amiibo.GetAreaSize(this->currentAreaAppId);
        if(sz == 0) return result::ResultAreaNotFound;
        amiibo.ReadArea(this->currentAreaAppId, data.GetPointer(), sz);
        data_size.SetValue(sz);
        return ams::ResultSuccess();
    }

    ams::Result IUser::SetApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle)
    {
        if(currentAreaAppId == 0) return result::ResultAreaNotFound;
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        amiibo.WriteArea(this->currentAreaAppId, (u8*)data.GetPointer(), data.GetSize());
        return ams::ResultSuccess();
    }

    ams::Result IUser::Flush(DeviceHandle handle)
    {
        this->deviceState = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    ams::Result IUser::Restore(DeviceHandle handle)
    {
        this->deviceState = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    ams::Result IUser::CreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(amiibo.ExistsArea(id)) return result::ResultAreaAlreadyCreated;
        amiibo.CreateArea(id, (u8*)data.GetPointer(), data.GetSize(), false);
        return ams::ResultSuccess();
    }

    ams::Result IUser::GetTagInfo(ams::sf::Out<TagInfo> out_info, DeviceHandle handle)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        TagInfo info = {};
        memcpy(&info, &amiibo.Infos.Tag, sizeof(info));
        if(amiibo.RandomizeUUID) randomGet(info.info.uuid, 10);
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result IUser::GetRegisterInfo(ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        RegisterInfo info = {};
        memcpy(&info, &amiibo.Infos.Register, sizeof(info));
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result IUser::GetModelInfo(ams::sf::Out<ModelInfo> out_info, DeviceHandle handle)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        ModelInfo info = {};
        memcpy(&info, &amiibo.Infos.Model, sizeof(info));
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result IUser::GetCommonInfo(ams::sf::Out<CommonInfo> out_info, DeviceHandle handle)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        CommonInfo info = {};
        memcpy(&info, &amiibo.Infos.Common, sizeof(info));
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result IUser::AttachActivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event)
    {
        event.SetValue(eventActivate.GetReadableHandle());
        return ams::ResultSuccess();
    }

    ams::Result IUser::AttachDeactivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event)
    {
        event.SetValue(eventDeactivate.GetReadableHandle());
        return ams::ResultSuccess();
    }

    ams::Result IUser::GetState(ams::sf::Out<u32> out_state)
    {
        out_state.SetValue(static_cast<u32>(this->state));
        return ams::ResultSuccess();
    }

    ams::Result IUser::GetDeviceState(DeviceHandle handle, ams::sf::Out<u32> out_state)
    {
        out_state.SetValue(static_cast<u32>(this->deviceState));
        return ams::ResultSuccess();
    }

    ams::Result IUser::GetNpadId(DeviceHandle handle, ams::sf::Out<u32> out_npad_id)
    {
        out_npad_id.SetValue(handle.npad_id);
        return ams::ResultSuccess();
    }

    ams::Result IUser::AttachAvailabilityChangeEvent(ams::sf::Out<ams::sf::CopyHandle> event)
    {
        event.SetValue(eventAvailabilityChange.GetReadableHandle());
        return ams::ResultSuccess();
    }

    ams::Result IUser::GetApplicationAreaSize(DeviceHandle handle, ams::sf::Out<u32> size)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(this->currentAreaAppId == 0) return result::ResultAreaNotFound;
        if(!amiibo.ExistsArea(this->currentAreaAppId)) return result::ResultAreaNotFound;
        size.SetValue(amiibo.GetAreaSize(this->currentAreaAppId));
        return ams::ResultSuccess();
    }

    ams::Result IUser::RecreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(amiibo.ExistsArea(id)) return result::ResultAreaAlreadyCreated;
        amiibo.CreateArea(id, (u8*)data.GetPointer(), data.GetSize(), true);
        return ams::ResultSuccess();
    }
}