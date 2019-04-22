 
#pragma once
#include <array>
#include <switch.h>
#include <stratosphere.hpp>
#include "nfp-utils.hpp"
#include "mii-shim.h"
#include "emu-amiibo.hpp"

enum NfpIUserCmd : u32
{
    NfpIUserCmd_Initialize = 0,
    NfpIUserCmd_Finalize = 1,
    NfpIUserCmd_ListDevices = 2,
    NfpIUserCmd_StartDetection = 3,
    NfpIUserCmd_StopDetection = 4,
    NfpIUserCmd_Mount = 5,
    NfpIUserCmd_Unmount = 6,
    NfpIUserCmd_OpenApplicationArea = 7,
    NfpIUserCmd_GetApplicationArea = 8,
    NfpIUserCmd_SetApplicationArea = 9,
    NfpIUserCmd_Flush = 10,
    NfpIUserCmd_Restore = 11,
    NfpIUserCmd_CreateApplicationArea = 12,
    NfpIUserCmd_GetTagInfo = 13,
    NfpIUserCmd_GetRegisterInfo = 14,
    NfpIUserCmd_GetCommonInfo = 15,
    NfpIUserCmd_GetModelInfo = 16,
    NfpIUserCmd_AttachActivateEvent = 17,
    NfpIUserCmd_AttachDeactivateEvent = 18,
    NfpIUserCmd_GetState = 19,
    NfpIUserCmd_GetDeviceState = 20,
    NfpIUserCmd_GetNpadId = 21,
    NfpIUserCmd_GetApplicationAreaSize = 22,
    NfpIUserCmd_AttachAvailabilityChangeEvent = 23,
    NfpIUserCmd_RecreateApplicationArea = 24,
};

class NfpIUser : public IServiceObject
{
    private:
        NfpuState state;
        NfpuDeviceState dvstate;
        IEvent* edeactivate;
        IEvent* eavailabilitychange;
        u32 cur_id;
    public:
        NfpIUser();
        ~NfpIUser();
    private:
        Result Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf);
        Result Finalize();
        Result ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u32> out_count);
        Result StartDetection(NfpDeviceHandle handle);
        Result StopDetection(NfpDeviceHandle handle);
        Result Mount(NfpDeviceHandle handle, NfpuDeviceType type, NfpuMountTarget target);
        Result Unmount(NfpDeviceHandle handle);
        Result OpenApplicationArea(Out<u32> npad_id, NfpDeviceHandle handle, u32 id);
        Result GetApplicationArea(OutBuffer<u8> data, Out<u32> data_size, NfpDeviceHandle handle);
        Result SetApplicationArea(InBuffer<u8> data, NfpDeviceHandle handle);
        Result Flush(NfpDeviceHandle handle);
        Result Restore(NfpDeviceHandle handle);
        Result CreateApplicationArea(InBuffer<u8> data, NfpDeviceHandle handle, u32 id);
        Result GetTagInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuTagInfo, 0x1> out_info);
        Result GetRegisterInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info);
        Result GetCommonInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuCommonInfo, 0x1> out_info);
        Result GetModelInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuModelInfo, 0x1> out_info);
        Result AttachActivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event);
        Result AttachDeactivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event);
        Result GetState(Out<u32> state);
        Result GetDeviceState(NfpDeviceHandle handle, Out<u32> state);
        Result GetNpadId(NfpDeviceHandle handle, Out<u32> npad_id);
        Result GetApplicationAreaSize(NfpDeviceHandle handle, Out<u32> size);
        Result AttachAvailabilityChangeEvent(Out<CopiedHandle> event);
        Result RecreateApplicationArea(InBuffer<u8> data, NfpDeviceHandle handle, u32 id);
    public:
        DEFINE_SERVICE_DISPATCH_TABLE
        {
            MakeServiceCommandMeta<NfpIUserCmd_Initialize, &NfpIUser::Initialize>(),
            MakeServiceCommandMeta<NfpIUserCmd_Finalize, &NfpIUser::Finalize>(),
            MakeServiceCommandMeta<NfpIUserCmd_ListDevices, &NfpIUser::ListDevices>(),
            MakeServiceCommandMeta<NfpIUserCmd_StartDetection, &NfpIUser::StartDetection>(),
            MakeServiceCommandMeta<NfpIUserCmd_StopDetection, &NfpIUser::StopDetection>(),
            MakeServiceCommandMeta<NfpIUserCmd_Mount, &NfpIUser::Mount>(),
            MakeServiceCommandMeta<NfpIUserCmd_Unmount, &NfpIUser::Unmount>(),
            MakeServiceCommandMeta<NfpIUserCmd_OpenApplicationArea, &NfpIUser::OpenApplicationArea>(),
            MakeServiceCommandMeta<NfpIUserCmd_GetApplicationArea, &NfpIUser::GetApplicationArea>(),
            MakeServiceCommandMeta<NfpIUserCmd_SetApplicationArea, &NfpIUser::SetApplicationArea>(),
            MakeServiceCommandMeta<NfpIUserCmd_Flush, &NfpIUser::Flush>(),
            MakeServiceCommandMeta<NfpIUserCmd_Restore, &NfpIUser::Restore>(),
            MakeServiceCommandMeta<NfpIUserCmd_CreateApplicationArea, &NfpIUser::CreateApplicationArea>(),
            MakeServiceCommandMeta<NfpIUserCmd_GetTagInfo, &NfpIUser::GetTagInfo>(),
            MakeServiceCommandMeta<NfpIUserCmd_GetRegisterInfo, &NfpIUser::GetRegisterInfo>(),
            MakeServiceCommandMeta<NfpIUserCmd_GetCommonInfo, &NfpIUser::GetCommonInfo>(),
            MakeServiceCommandMeta<NfpIUserCmd_GetModelInfo, &NfpIUser::GetModelInfo>(),
            MakeServiceCommandMeta<NfpIUserCmd_AttachActivateEvent, &NfpIUser::AttachActivateEvent>(),
            MakeServiceCommandMeta<NfpIUserCmd_AttachDeactivateEvent, &NfpIUser::AttachDeactivateEvent>(),
            MakeServiceCommandMeta<NfpIUserCmd_GetState, &NfpIUser::GetState>(),
            MakeServiceCommandMeta<NfpIUserCmd_GetDeviceState, &NfpIUser::GetDeviceState>(),
            MakeServiceCommandMeta<NfpIUserCmd_GetNpadId, &NfpIUser::GetNpadId>(),
            MakeServiceCommandMeta<NfpIUserCmd_AttachAvailabilityChangeEvent, &NfpIUser::AttachAvailabilityChangeEvent>(),
            MakeServiceCommandMeta<NfpIUserCmd_GetApplicationAreaSize, &NfpIUser::GetApplicationAreaSize>(),
            MakeServiceCommandMeta<NfpIUserCmd_RecreateApplicationArea, &NfpIUser::RecreateApplicationArea, FirmwareVersion_500>(), /* TODO: What version was this added in? */
        };
};
