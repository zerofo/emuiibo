 
#pragma once
#include <array>
#include <switch.h>
#include <stratosphere.hpp>
#include "nfp-utils.hpp"
#include "mii-shim.h"
#include "emu-amiibo.hpp"

enum NfpISystemCmd : u32
{
    NfpISystemCmd_Initialize = 0,
    NfpISystemCmd_Finalize = 1,
    NfpISystemCmd_ListDevices = 2,
    NfpISystemCmd_StartDetection = 3,
    NfpISystemCmd_StopDetection = 4,
    NfpISystemCmd_Mount = 5,
    NfpISystemCmd_Unmount = 6,
    NfpISystemCmd_Flush = 10,
    NfpISystemCmd_Restore = 11,
    NfpISystemCmd_GetTagInfo = 13,
    NfpISystemCmd_GetRegisterInfo = 14,
    NfpISystemCmd_GetCommonInfo = 15,
    NfpISystemCmd_GetModelInfo = 16,
    NfpISystemCmd_AttachActivateEvent = 17,
    NfpISystemCmd_AttachDeactivateEvent = 18,
    NfpISystemCmd_GetState = 19,
    NfpISystemCmd_GetDeviceState = 20,
    NfpISystemCmd_GetNpadId = 21,
    NfpISystemCmd_AttachAvailabilityChangeEvent = 23,
    NfpISystemCmd_Format = 100,
    NfpISystemCmd_GetAdminInfo = 101,
    NfpISystemCmd_GetRegisterInfo2 = 102,
    NfpISystemCmd_SetRegisterInfo = 103,
    NfpISystemCmd_DeleteRegisterInfo = 104,
    NfpISystemCmd_DeleteApplicationArea = 105,
    NfpISystemCmd_ExistsApplicationArea = 106,
};

struct AdminInfo
{
    u8 data[0x40];
} PACKED;

class NfpISystem : public IServiceObject
{
    private:
        NfpuState state;
        NfpuDeviceState dvstate;
        IEvent* edeactivate;
        IEvent* eavailabilitychange;
        u32 cur_id;
    public:
        NfpISystem();
        ~NfpISystem();
    private:
        Result Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf);
        Result Finalize();
        Result ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u32> out_count);
        Result StartDetection(NfpDeviceHandle handle);
        Result StopDetection(NfpDeviceHandle handle);
        Result Mount(NfpDeviceHandle handle, NfpuDeviceType type, NfpuMountTarget target);
        Result Unmount(NfpDeviceHandle handle);
        Result Flush(NfpDeviceHandle handle);
        Result Restore(NfpDeviceHandle handle);
        Result GetTagInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuTagInfo, 0x1> out_info);
        Result GetRegisterInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info);
        Result GetCommonInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuCommonInfo, 0x1> out_info);
        Result GetModelInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuModelInfo, 0x1> out_info);
        Result AttachActivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event);
        Result AttachDeactivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event);
        Result GetState(Out<u32> state);
        Result GetDeviceState(NfpDeviceHandle handle, Out<u32> state);
        Result GetNpadId(NfpDeviceHandle handle, Out<u32> npad_id);
        Result AttachAvailabilityChangeEvent(Out<CopiedHandle> event);
        Result Format(NfpDeviceHandle handle);
        Result GetAdminInfo(NfpDeviceHandle handle, OutPointerWithServerSize<AdminInfo, 0x1> info);
        Result GetRegisterInfo2(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info);
        Result SetRegisterInfo(NfpDeviceHandle handle, InPointer<NfpuRegisterInfo> info);
        Result DeleteRegisterInfo(NfpDeviceHandle handle);
        Result DeleteApplicationArea(NfpDeviceHandle handle);
        Result ExistsApplicationArea(NfpDeviceHandle handle, Out<u8> exists);
    public:
        DEFINE_SERVICE_DISPATCH_TABLE
        {
            MakeServiceCommandMeta<NfpISystemCmd_Initialize, &NfpISystem::Initialize>(),
            MakeServiceCommandMeta<NfpISystemCmd_Finalize, &NfpISystem::Finalize>(),
            MakeServiceCommandMeta<NfpISystemCmd_ListDevices, &NfpISystem::ListDevices>(),
            MakeServiceCommandMeta<NfpISystemCmd_StartDetection, &NfpISystem::StartDetection>(),
            MakeServiceCommandMeta<NfpISystemCmd_StopDetection, &NfpISystem::StopDetection>(),
            MakeServiceCommandMeta<NfpISystemCmd_Mount, &NfpISystem::Mount>(),
            MakeServiceCommandMeta<NfpISystemCmd_Unmount, &NfpISystem::Unmount>(),
            MakeServiceCommandMeta<NfpISystemCmd_Flush, &NfpISystem::Flush>(),
            MakeServiceCommandMeta<NfpISystemCmd_Restore, &NfpISystem::Restore>(),
            MakeServiceCommandMeta<NfpISystemCmd_GetTagInfo, &NfpISystem::GetTagInfo>(),
            MakeServiceCommandMeta<NfpISystemCmd_GetRegisterInfo, &NfpISystem::GetRegisterInfo>(),
            MakeServiceCommandMeta<NfpISystemCmd_GetCommonInfo, &NfpISystem::GetCommonInfo>(),
            MakeServiceCommandMeta<NfpISystemCmd_GetModelInfo, &NfpISystem::GetModelInfo>(),
            MakeServiceCommandMeta<NfpISystemCmd_AttachActivateEvent, &NfpISystem::AttachActivateEvent>(),
            MakeServiceCommandMeta<NfpISystemCmd_AttachDeactivateEvent, &NfpISystem::AttachDeactivateEvent>(),
            MakeServiceCommandMeta<NfpISystemCmd_GetState, &NfpISystem::GetState>(),
            MakeServiceCommandMeta<NfpISystemCmd_GetDeviceState, &NfpISystem::GetDeviceState>(),
            MakeServiceCommandMeta<NfpISystemCmd_GetNpadId, &NfpISystem::GetNpadId>(),
            MakeServiceCommandMeta<NfpISystemCmd_AttachAvailabilityChangeEvent, &NfpISystem::AttachAvailabilityChangeEvent>(),
            MakeServiceCommandMeta<NfpISystemCmd_Format, &NfpISystem::Format>(),
            MakeServiceCommandMeta<NfpISystemCmd_GetAdminInfo, &NfpISystem::GetAdminInfo>(),
            MakeServiceCommandMeta<NfpISystemCmd_GetRegisterInfo2, &NfpISystem::GetRegisterInfo2>(),
            MakeServiceCommandMeta<NfpISystemCmd_SetRegisterInfo, &NfpISystem::SetRegisterInfo>(),
            MakeServiceCommandMeta<NfpISystemCmd_DeleteRegisterInfo, &NfpISystem::DeleteRegisterInfo>(),
            MakeServiceCommandMeta<NfpISystemCmd_DeleteApplicationArea, &NfpISystem::DeleteApplicationArea>(),
            MakeServiceCommandMeta<NfpISystemCmd_ExistsApplicationArea, &NfpISystem::ExistsApplicationArea>(),
        };
};
