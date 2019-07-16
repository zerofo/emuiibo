#include <nfp/user/user_IUser.hpp>

#include <atomic>
#include <cstdio>
#include <array>
#include <sys/stat.h>
#include <switch.h>
#include <emu/emu_Emulation.hpp>

namespace nfp::user
{
    // #define LogCommand do { FILE *f = fopen("sdmc:/emuiibo-cmd.log", "a"); fprintf(f, "%s\n", __PRETTY_FUNCTION__); fclose(f); } while(0)

    IUser::IUser()
    {
        currentAreaAppId = 0;
        eventActivate = CreateWriteOnlySystemEvent();
        eventDeactivate = CreateWriteOnlySystemEvent();
        eventAvailabilityChange = CreateWriteOnlySystemEvent();
        state = NfpuState_NonInitialized;
        deviceState = NfpuDeviceState_Unavailable;
    }

    IUser::~IUser()
    {
        delete eventActivate;
        delete eventDeactivate;
        delete eventAvailabilityChange;
    }

    Result IUser::Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf)
    {
        state = NfpuState_Initialized;
        deviceState = NfpuDeviceState_Initialized;
        return 0;
    }

    Result IUser::Finalize()
    {
        state = NfpuState_NonInitialized;
        deviceState = NfpuDeviceState_Finalized;
        return 0;
    }

    Result IUser::ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u32> out_count)
    {
        u64 dvcid = 0x20;
        hidScanInput();
        if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) dvcid = (u64)CONTROLLER_PLAYER_1;
        memcpy(out_devices.pointer, &dvcid, sizeof(u64));
        out_count.SetValue(1);
        return 0;
    }

    Result IUser::StartDetection(DeviceHandle handle)
    {
        eventActivate->Signal();
        eventAvailabilityChange->Signal();
        deviceState = NfpuDeviceState_TagFound;
        return 0;
    }

    Result IUser::StopDetection(DeviceHandle handle)
    {
        switch(deviceState)
        {
            case NfpuDeviceState_TagFound:
            case NfpuDeviceState_TagMounted:
                eventDeactivate->Signal();
            case NfpuDeviceState_SearchingForTag:
            case NfpuDeviceState_TagRemoved:
                deviceState = NfpuDeviceState_Initialized;
                break;
            default:
                break;
        }
        return 0;
    }

    Result IUser::Mount(DeviceHandle handle, u32 type, u32 target)
    {
        eventActivate->Signal();
        deviceState = NfpuDeviceState_TagMounted;
        return 0;
    }

    Result IUser::Unmount(DeviceHandle handle)
    {
        eventDeactivate->Signal();
        deviceState = NfpuDeviceState_SearchingForTag;
        return 0;
    }

    Result IUser::OpenApplicationArea(Out<u32> npad_id, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(amiibo.ExistsArea(id))
        {
            currentAreaAppId = id;
            npad_id.SetValue(handle.NpadId);
            return 0;
        }
        return result::ResultAreaNotFound;
    }

    Result IUser::GetApplicationArea(OutBuffer<u8> data, Out<u32> data_size, DeviceHandle handle)
    {
        if(currentAreaAppId == 0) return result::ResultAreaNotFound;
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        u64 sz = (u64)amiibo.GetAreaSize(currentAreaAppId);
        if(sz == 0) return result::ResultAreaNotFound;
        amiibo.ReadArea(currentAreaAppId, data.buffer, sz);
        data_size.SetValue(sz);
        return 0;
    }

    Result IUser::SetApplicationArea(InBuffer<u8> data, DeviceHandle handle)
    {
        if(currentAreaAppId == 0) return result::ResultAreaNotFound;
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        amiibo.WriteArea(currentAreaAppId, data.buffer, data.num_elements);
        return 0;
    }

    Result IUser::Flush(DeviceHandle handle)
    {
        deviceState = NfpuDeviceState_TagFound;
        return 0;
    }

    Result IUser::Restore(DeviceHandle handle)
    {
        deviceState = NfpuDeviceState_TagFound;
        return 0;
    }

    Result IUser::CreateApplicationArea(InBuffer<u8> data, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(amiibo.ExistsArea(id)) return result::ResultAreaAlreadyCreated;
        amiibo.CreateArea(id, data.buffer, data.num_elements, false);
        return 0;
    }

    Result IUser::GetTagInfo(DeviceHandle handle, OutPointerWithServerSize<TagInfo, 1> out_info)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Tag, sizeof(TagInfo));
        if(amiibo.RandomizeUUID) randomGet(out_info.pointer->uuid, 10);
        return 0;
    }

    Result IUser::GetRegisterInfo(DeviceHandle handle, OutPointerWithServerSize<RegisterInfo, 1> out_info)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Register, sizeof(RegisterInfo));
        return 0;
    }

    Result IUser::GetModelInfo(DeviceHandle handle, OutPointerWithServerSize<ModelInfo, 1> out_info)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Model, sizeof(ModelInfo));
        return 0;
    }

    Result IUser::GetCommonInfo(DeviceHandle handle, OutPointerWithServerSize<CommonInfo, 1> out_info)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Common, sizeof(CommonInfo));
        return 0;
    }

    Result IUser::AttachActivateEvent(DeviceHandle handle, Out<CopiedHandle> event)
    {
        event.SetValue(eventActivate->GetHandle());
        return 0;
    }

    Result IUser::AttachDeactivateEvent(DeviceHandle handle, Out<CopiedHandle> event)
    {
        event.SetValue(eventDeactivate->GetHandle());
        return 0;
    }

    Result IUser::GetState(Out<u32> out_state)
    {
        out_state.SetValue(static_cast<u32>(state));
        return 0;
    }

    Result IUser::GetDeviceState(DeviceHandle handle, Out<u32> out_state)
    {
        out_state.SetValue(static_cast<u32>(deviceState));
        return 0;
    }

    Result IUser::GetNpadId(DeviceHandle handle, Out<u32> out_npad_id)
    {
        out_npad_id.SetValue(handle.NpadId);
        return 0;
    }

    Result IUser::AttachAvailabilityChangeEvent(Out<CopiedHandle> event)
    {
        event.SetValue(eventAvailabilityChange->GetHandle());
        return 0;
    }

    Result IUser::GetApplicationAreaSize(DeviceHandle handle, Out<u32> size)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(currentAreaAppId == 0) return result::ResultAreaNotFound;
        if(!amiibo.ExistsArea(currentAreaAppId)) return result::ResultAreaNotFound;
        size.SetValue(amiibo.GetAreaSize(currentAreaAppId));
        return 0;
    }

    Result IUser::RecreateApplicationArea(InBuffer<u8> data, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(amiibo.ExistsArea(id)) return result::ResultAreaAlreadyCreated;
        amiibo.CreateArea(id, data.buffer, data.num_elements, true);
        return 0;
    }
}