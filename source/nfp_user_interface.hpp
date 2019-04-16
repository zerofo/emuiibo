/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#pragma once
#include <array>
#include <switch.h>
#include <stratosphere.hpp>
#include "nfp_shim.h"
#include "mii-shim.h"
#include "emu-amiibo.hpp"

enum NfpUserInterfaceCmd : u32 {
    NfpUserInterfaceCmd_Initialize = 0,
    NfpUserInterfaceCmd_Finalize = 1,
    NfpUserInterfaceCmd_ListDevices = 2,
    NfpUserInterfaceCmd_StartDetection = 3,
    NfpUserInterfaceCmd_StopDetection = 4,
    NfpUserInterfaceCmd_Mount = 5,
    NfpUserInterfaceCmd_Unmount = 6,
    NfpUserInterfaceCmd_OpenApplicationArea = 7,
    NfpUserInterfaceCmd_GetApplicationArea = 8,
    NfpUserInterfaceCmd_SetApplicationArea = 9,
    NfpUserInterfaceCmd_Flush = 10,
    NfpUserInterfaceCmd_Restore = 11,
    NfpUserInterfaceCmd_CreateApplicationArea = 12,
    NfpUserInterfaceCmd_GetTagInfo = 13,
    NfpUserInterfaceCmd_GetRegisterInfo = 14,
    NfpUserInterfaceCmd_GetCommonInfo = 15,
    NfpUserInterfaceCmd_GetModelInfo = 16,
    NfpUserInterfaceCmd_AttachActivateEvent = 17,
    NfpUserInterfaceCmd_AttachDeactivateEvent = 18,
    NfpUserInterfaceCmd_GetState = 19,
    NfpUserInterfaceCmd_GetDeviceState = 20,
    NfpUserInterfaceCmd_GetNpadId = 21,
    NfpUserInterfaceCmd_GetApplicationAreaSize = 22,
    NfpUserInterfaceCmd_AttachAvailabilityChangeEvent = 23,
    NfpUserInterfaceCmd_RecreateApplicationArea = 24,
};

enum class State : u32 {
    NonInitialized = 0,
    Initialized = 1,
};

enum class DeviceState : u32 {
    Initialized = 0,
    SearchingForTag = 1,
    TagFound = 2,
    TagRemoved = 3,
    TagNearby = 4,
    Unknown5 = 5,
    Finalized = 6
};

class NfpResults {
    public:
        static constexpr Result ResultNeedRestart = MAKERESULT(115, 96);
        static constexpr Result ResultDeviceNotFound = MAKERESULT(115, 64);
        static constexpr Result ResultNotFoundArea = MAKERESULT(115, 128);
};

class NfpUserInterface : public IServiceObject {
    private:
        NfpUser *forward_intf;
        NfpuState state;
        NfpuDeviceState dvstate;
        IEvent* edeactivate;
        IEvent* eavailabilitychange;
    public:
        NfpUserInterface(NfpUser *u);
        ~NfpUserInterface();
        static constexpr u64 EmuDeviceHandle = 0xfeedbeef; // emuiibo's custom handle
    private:
        Result Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf);
        Result Finalize();
        Result ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u64> out_count);
        Result StartDetection(u64 handle);
        Result StopDetection(u64 handle);
        Result Mount(u64 handle, NfpuDeviceType type, NfpuMountTarget target);
        Result Unmount(u64 handle);
        Result OpenApplicationArea(u64 handle, u32 access_id);
        Result GetApplicationArea(u64 handle, OutBuffer<u8> out_area, Out<u32> out_area_size);
        Result SetApplicationArea(u64 handle, InBuffer<u8> area);
        Result Flush(u64 handle);
        Result Restore(u64 handle);
        Result CreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area);
        Result GetTagInfo(u64 handle, OutPointerWithServerSize<NfpuTagInfo, 0x1> out_info);
        Result GetRegisterInfo(u64 handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info);
        Result GetCommonInfo(u64 handle, OutPointerWithServerSize<NfpuCommonInfo, 0x1> out_info);
        Result GetModelInfo(u64 handle, OutPointerWithServerSize<NfpuModelInfo, 0x1> out_info);
        Result AttachActivateEvent(u64 handle, Out<CopiedHandle> event);
        Result AttachDeactivateEvent(u64 handle, Out<CopiedHandle> event);
        Result GetState(Out<u32> state);
        Result GetDeviceState(u64 handle, Out<u32> state);
        Result GetNpadId(u64 handle, Out<u32> npad_id);
        Result AttachAvailabilityChangeEvent(Out<CopiedHandle> event);
        Result GetApplicationAreaSize(Out<u32> size);
        Result RecreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area);
    public:
        DEFINE_SERVICE_DISPATCH_TABLE {
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Initialize, &NfpUserInterface::Initialize>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Finalize, &NfpUserInterface::Finalize>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_ListDevices, &NfpUserInterface::ListDevices>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_StartDetection, &NfpUserInterface::StartDetection>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_StopDetection, &NfpUserInterface::StopDetection>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Mount, &NfpUserInterface::Mount>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Unmount, &NfpUserInterface::Unmount>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_OpenApplicationArea, &NfpUserInterface::OpenApplicationArea>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetApplicationArea, &NfpUserInterface::GetApplicationArea>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_SetApplicationArea, &NfpUserInterface::SetApplicationArea>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Flush, &NfpUserInterface::Flush>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_Restore, &NfpUserInterface::Restore>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_CreateApplicationArea, &NfpUserInterface::CreateApplicationArea>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetTagInfo, &NfpUserInterface::GetTagInfo>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetRegisterInfo, &NfpUserInterface::GetRegisterInfo>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetCommonInfo, &NfpUserInterface::GetCommonInfo>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetModelInfo, &NfpUserInterface::GetModelInfo>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_AttachActivateEvent, &NfpUserInterface::AttachActivateEvent>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_AttachDeactivateEvent, &NfpUserInterface::AttachDeactivateEvent>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetState, &NfpUserInterface::GetState>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetDeviceState, &NfpUserInterface::GetDeviceState>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetNpadId, &NfpUserInterface::GetNpadId>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_AttachAvailabilityChangeEvent, &NfpUserInterface::AttachAvailabilityChangeEvent>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_GetApplicationAreaSize, &NfpUserInterface::GetApplicationAreaSize>(),
            MakeServiceCommandMeta<NfpUserInterfaceCmd_RecreateApplicationArea, &NfpUserInterface::RecreateApplicationArea, FirmwareVersion_500>(), /* TODO: What version was this added in? */
        };
};
