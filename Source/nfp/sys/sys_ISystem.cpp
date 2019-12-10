#include <nfp/sys/sys_ISystem.hpp>

#include <atomic>
#include <cstdio>
#include <array>
#include <sys/stat.h>
#include <switch.h>
#include <emu/emu_Emulation.hpp>

namespace nfp::sys
{
    // #define LogCommand do { FILE *f = fopen("sdmc:/emuiibo-cmd.log", "a"); fprintf(f, "%s\n", __PRETTY_FUNCTION__); fclose(f); } while(0)

    ISystem::ISystem()
    {
        currentAreaAppId = 0;
        eventActivate = CreateWriteOnlySystemEvent();
        eventDeactivate = CreateWriteOnlySystemEvent();
        eventAvailabilityChange = CreateWriteOnlySystemEvent();
        state = NfpState_NonInitialized;
        deviceState = NfpDeviceState_Unavailable;
    }

    ISystem::~ISystem()
    {
        delete eventActivate;
        delete eventDeactivate;
        delete eventAvailabilityChange;
    }

    Result ISystem::Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf)
    {
        state = NfpState_Initialized;
        deviceState = NfpDeviceState_Initialized;
        return ams::ResultSuccess();
    }

    Result ISystem::Finalize()
    {
        state = NfpState_NonInitialized;
        deviceState = NfpDeviceState_Finalized;
        return ams::ResultSuccess();
    }

    Result ISystem::ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u32> out_count)
    {
        u64 dvcid = 0x20;
        hidScanInput();
        if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) dvcid = (u64)CONTROLLER_PLAYER_1;
        memcpy(out_devices.pointer, &dvcid, sizeof(u64));
        out_count.SetValue(1);
        return ams::ResultSuccess();
    }

    Result ISystem::StartDetection(DeviceHandle handle)
    {
        eventActivate->Signal();
        eventAvailabilityChange->Signal();
        deviceState = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    Result ISystem::StopDetection(DeviceHandle handle)
    {
        switch(deviceState)
        {
            case NfpDeviceState_TagFound:
            case NfpDeviceState_TagMounted:
                eventDeactivate->Signal();
            case NfpDeviceState_SearchingForTag:
            case NfpDeviceState_TagRemoved:
                deviceState = NfpDeviceState_Initialized;
                break;
            default:
                break;
        }
        return ams::ResultSuccess();
    }

    Result ISystem::Mount(DeviceHandle handle, u32 type, u32 target)
    {
        eventActivate->Signal();
        deviceState = NfpDeviceState_TagMounted;
        return ams::ResultSuccess();
    }

    Result ISystem::Unmount(DeviceHandle handle)
    {
        eventDeactivate->Signal();
        deviceState = NfpDeviceState_SearchingForTag;
        return ams::ResultSuccess();
    }

    Result ISystem::OpenApplicationArea(Out<u32> npad_id, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(amiibo.ExistsArea(id))
        {
            currentAreaAppId = id;
            npad_id.SetValue(handle.NpadId);
            return ams::ResultSuccess();
        }
        return result::ResultAreaNotFound;
    }

    Result ISystem::GetApplicationArea(OutBuffer<u8> data, Out<u32> data_size, DeviceHandle handle)
    {
        if(currentAreaAppId == 0) return result::ResultAreaNotFound;
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        u64 sz = (u64)amiibo.GetAreaSize(currentAreaAppId);
        if(sz == 0) return result::ResultAreaNotFound;
        amiibo.ReadArea(currentAreaAppId, data.buffer, sz);
        data_size.SetValue(sz);
        return ams::ResultSuccess();
    }

    Result ISystem::SetApplicationArea(InBuffer<u8> data, DeviceHandle handle)
    {
        if(currentAreaAppId == 0) return result::ResultAreaNotFound;
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        amiibo.WriteArea(currentAreaAppId, data.buffer, data.num_elements);
        return ams::ResultSuccess();
    }

    Result ISystem::Flush(DeviceHandle handle)
    {
        deviceState = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    Result ISystem::Restore(DeviceHandle handle)
    {
        deviceState = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    Result ISystem::CreateApplicationArea(InBuffer<u8> data, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(amiibo.ExistsArea(id)) return result::ResultAreaAlreadyCreated;
        amiibo.CreateArea(id, data.buffer, data.num_elements, false);
        return ams::ResultSuccess();
    }

    Result ISystem::GetTagInfo(DeviceHandle handle, OutPointerWithServerSize<TagInfo, 1> out_info)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Tag, sizeof(TagInfo));
        return ams::ResultSuccess();
    }

    Result ISystem::GetRegisterInfo(DeviceHandle handle, OutPointerWithServerSize<RegisterInfo, 1> out_info)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Register, sizeof(RegisterInfo));
        return ams::ResultSuccess();
    }

    Result ISystem::GetModelInfo(DeviceHandle handle, OutPointerWithServerSize<ModelInfo, 1> out_info)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Model, sizeof(ModelInfo));
        return ams::ResultSuccess();
    }

    Result ISystem::GetCommonInfo(DeviceHandle handle, OutPointerWithServerSize<CommonInfo, 1> out_info)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Common, sizeof(CommonInfo));
        return ams::ResultSuccess();
    }

    Result ISystem::AttachActivateEvent(DeviceHandle handle, Out<CopiedHandle> event)
    {
        event.SetValue(eventActivate->GetHandle());
        return ams::ResultSuccess();
    }

    Result ISystem::AttachDeactivateEvent(DeviceHandle handle, Out<CopiedHandle> event)
    {
        event.SetValue(eventDeactivate->GetHandle());
        return ams::ResultSuccess();
    }

    Result ISystem::GetState(Out<u32> out_state)
    {
        out_state.SetValue(static_cast<u32>(state));
        return ams::ResultSuccess();
    }

    Result ISystem::GetDeviceState(DeviceHandle handle, Out<u32> out_state)
    {
        out_state.SetValue(static_cast<u32>(deviceState));
        return ams::ResultSuccess();
    }

    Result ISystem::GetNpadId(DeviceHandle handle, Out<u32> out_npad_id)
    {
        out_npad_id.SetValue(handle.NpadId);
        return ams::ResultSuccess();
    }

    Result ISystem::AttachAvailabilityChangeEvent(Out<CopiedHandle> event)
    {
        event.SetValue(eventAvailabilityChange->GetHandle());
        return ams::ResultSuccess();
    }

    Result ISystem::GetApplicationAreaSize(DeviceHandle handle, Out<u32> size)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(currentAreaAppId == 0) return result::ResultAreaNotFound;
        if(!amiibo.ExistsArea(currentAreaAppId)) return result::ResultAreaNotFound;
        size.SetValue(amiibo.GetAreaSize(currentAreaAppId));
        return ams::ResultSuccess();
    }

    Result ISystem::RecreateApplicationArea(InBuffer<u8> data, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(amiibo.ExistsArea(id)) return result::ResultAreaAlreadyCreated;
        amiibo.CreateArea(id, data.buffer, data.num_elements, true);
        return ams::ResultSuccess();
    }

    Result ISystem::Format(DeviceHandle handle)
    {
        return ams::ResultSuccess(); // Format...??
    }

    Result ISystem::GetAdminInfo(DeviceHandle handle, OutPointerWithServerSize<AdminInfo, 1> out_info)
    {
        
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Admin, sizeof(AdminInfo));

        return ams::ResultSuccess();
    }

    Result ISystem::GetRegisterInfo2(DeviceHandle handle, OutPointerWithServerSize<RegisterInfo, 1> out_info)
    {
        
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        if(!amiibo.IsValid()) return result::ResultDeviceNotFound;
        memcpy(out_info.pointer, &amiibo.Infos.Register, sizeof(RegisterInfo));

        return ams::ResultSuccess();
    }

    Result ISystem::SetRegisterInfo(DeviceHandle handle, InPointer<RegisterInfo> info)
    {
        return ams::ResultSuccess(); // TODO
    }

    Result ISystem::DeleteRegisterInfo(DeviceHandle handle)
    {
        return ams::ResultSuccess(); // TODO
    }

    Result ISystem::DeleteApplicationArea(DeviceHandle handle)
    {
        return ams::ResultSuccess(); // TODO
    }

    Result ISystem::ExistsApplicationArea(DeviceHandle handle, Out<u8> exists)
    {
        exists.SetValue(1);
        return ams::ResultSuccess(); // TODO
    }
}