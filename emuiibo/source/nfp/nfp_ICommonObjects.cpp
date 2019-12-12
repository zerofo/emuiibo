#include "nfp/nfp_ICommonObjects.hpp"
#include "emu/emu_Emulation.hpp"
#include <atomic>
#include <cstdio>
#include <sys/stat.h>

namespace nfp
{
    ICommonInterface::ICommonInterface(Service *fwd)
    {
        this->currentAreaAppId = 0;
        this->eventActivate.InitializeAsInterProcessEvent();
        this->eventDeactivate.InitializeAsInterProcessEvent();
        this->eventAvailabilityChange.InitializeAsInterProcessEvent();
        this->state = NfpState_NonInitialized;
        this->deviceState = NfpDeviceState_Unavailable;
        this->fwd_srv = fwd;
    }

    ICommonInterface::~ICommonInterface()
    {
        serviceClose(this->fwd_srv);
        delete this->fwd_srv;
    }

    ams::Result ICommonInterface::Initialize(const ams::sf::ClientAppletResourceUserId &client_aruid, const ams::sf::ClientProcessId &client_pid, const ams::sf::InBuffer &mcu_data)
    {
        LOG_FMT("Process ID: 0x" << std::hex << client_pid.GetValue().value << ", ARUID: 0x" << std:: hex << client_aruid.GetValue().value)
        this->state = NfpState_Initialized;
        this->deviceState = NfpDeviceState_Initialized;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::Finalize()
    {
        LOG_FMT("Finalizing...")
        this->state = NfpState_NonInitialized;
        this->deviceState = NfpDeviceState_Finalized;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::ListDevices(const ams::sf::OutPointerArray<DeviceHandle> &out_devices, ams::sf::Out<s32> out_count)
    {
        LOG_FMT("Device array length: " << out_devices.GetSize())
        u64 id = 0x20;
        hidScanInput();
        if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) id = (u64)CONTROLLER_PLAYER_1;
        DeviceHandle handle = {};
        handle.npad_id = (u32)id;
        out_devices[0] = handle;
        out_count.SetValue(1);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::StartDetection(DeviceHandle handle)
    {
        LOG_FMT("Started detection")
        this->eventActivate.Signal();
        this->eventAvailabilityChange.Signal();
        this->deviceState = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::StopDetection(DeviceHandle handle)
    {
        LOG_FMT("Stopped detection")
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

    ams::Result ICommonInterface::Mount(DeviceHandle handle, u32 type, u32 target)
    {
        LOG_FMT("Mounted")
        this->eventActivate.Signal();
        this->deviceState = NfpDeviceState_TagMounted;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::Unmount(DeviceHandle handle)
    {
        LOG_FMT("Unmounted")
        this->eventDeactivate.Signal();
        this->deviceState = NfpDeviceState_SearchingForTag;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::Flush(DeviceHandle handle)
    {
        LOG_FMT("Flushed")
        this->deviceState = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::Restore(DeviceHandle handle)
    {
        LOG_FMT("Restored")
        this->deviceState = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetTagInfo(ams::sf::Out<TagInfo> out_info, DeviceHandle handle)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        LOG_FMT("Tag info - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", amiibo name: " << amiibo.Infos.Register.info.amiibo_name)
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        TagInfo info = {};
        memcpy(&info, &amiibo.Infos.Tag, sizeof(info));
        if(amiibo.RandomizeUUID) randomGet(info.info.uuid, 10);
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetRegisterInfo(ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        LOG_FMT("Model info - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", amiibo name: " << amiibo.Infos.Register.info.amiibo_name)
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        RegisterInfo info = {};
        memcpy(&info, &amiibo.Infos.Register, sizeof(info));
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetModelInfo(ams::sf::Out<ModelInfo> out_info, DeviceHandle handle)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        LOG_FMT("Model info - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", amiibo name: " << amiibo.Infos.Register.info.amiibo_name)
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        ModelInfo info = {};
        memcpy(&info, &amiibo.Infos.Model, sizeof(info));
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetCommonInfo(ams::sf::Out<CommonInfo> out_info, DeviceHandle handle)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        LOG_FMT("Model info - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", amiibo name: " << amiibo.Infos.Register.info.amiibo_name)
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        CommonInfo info = {};
        memcpy(&info, &amiibo.Infos.Common, sizeof(info));
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::AttachActivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event)
    {
        event.SetValue(eventActivate.GetReadableHandle());
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::AttachDeactivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event)
    {
        event.SetValue(eventDeactivate.GetReadableHandle());
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetState(ams::sf::Out<u32> out_state)
    {
        out_state.SetValue(static_cast<u32>(this->state));
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetDeviceState(DeviceHandle handle, ams::sf::Out<u32> out_state)
    {
        out_state.SetValue(static_cast<u32>(this->deviceState));
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetNpadId(DeviceHandle handle, ams::sf::Out<u32> out_npad_id)
    {
        out_npad_id.SetValue(handle.npad_id);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::AttachAvailabilityChangeEvent(ams::sf::Out<ams::sf::CopyHandle> event)
    {
        event.SetValue(eventAvailabilityChange.GetReadableHandle());
        return ams::ResultSuccess();
    }

    static Result _fwd_CreateInterface(Service *out, Service *manager_srv)
    {
        return serviceDispatch(manager_srv, 0,
            .out_num_objects = 1,
            .out_objects = out,
        );
    }

    ams::Result ICommonManager::CreateForwardInterface(Service *manager, Service *out)
    {
        if(emu::IsStatusOff()) LOG_FMT("Status isn't on - skipping MitM...")
        R_UNLESS(emu::IsStatusOn(), ams::sm::mitm::ResultShouldForwardToSession());
        R_TRY(_fwd_CreateInterface(out, manager));
        LOG_FMT("Created MitM'd interface for emuiibo!")
        if(emu::IsStatusOnOnce()) emu::SetStatus(emu::EmulationStatus::Off);
        return ams::ResultSuccess();
    }
}