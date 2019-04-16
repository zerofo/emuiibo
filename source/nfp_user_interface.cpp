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

#include <atomic>
#include <cstdio>
#include <array>
#include <switch.h>

#include "nfp_user_interface.hpp"

extern std::atomic_bool g_comboDetected;
extern IEvent *g_eactivate;
extern FILE *g_logging_file;

HosMutex g_comboLock;

NfpUserInterface::NfpUserInterface(NfpUser *u)
{
    this->forward_intf = u;
    edeactivate = CreateWriteOnlySystemEvent();
    eavailabilitychange = CreateWriteOnlySystemEvent();
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Unavailable;
}

NfpUserInterface::~NfpUserInterface()
{
    delete edeactivate;
    delete eavailabilitychange;
    nfpUserClose(this->forward_intf);
    delete this->forward_intf;
}

Result NfpUserInterface::Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf)
{
    state = NfpuState_Initialized;
    dvstate = NfpuDeviceState_Initialized;
    return 0;
}

Result NfpUserInterface::Finalize()
{
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Finalized;
    return 0;
}

Result NfpUserInterface::ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u64> out_count)
{
    if(out_devices.num_elements >= 1)
    {
        memcpy(out_devices.pointer, &EmuDeviceHandle, sizeof(EmuDeviceHandle));
        out_count.SetValue(1);
    }
    else out_count.SetValue(0);
    return 0;
}

Result NfpUserInterface::StartDetection(u64 handle)
{
    if((dvstate == NfpuDeviceState_Initialized) || (dvstate == NfpuDeviceState_TagRemoved))
    {
        std::scoped_lock<HosMutex> lck(g_comboLock);
        if(g_comboDetected) {
            eavailabilitychange->Signal();
            g_comboDetected = false;
        }
        dvstate = NfpuDeviceState_TagFound;
        return 0;
    }
    return NfpResults::ResultDeviceNotFound;
}

Result NfpUserInterface::StopDetection(u64 handle)
{
    switch(dvstate)
    {
        case NfpuDeviceState_TagFound:
        case NfpuDeviceState_TagMounted:
            edeactivate->Signal();
            dvstate = NfpuDeviceState_Initialized;
            break;
        case NfpuDeviceState_SearchingForTag:
        case NfpuDeviceState_TagRemoved:
            dvstate = NfpuDeviceState_Initialized;
            break;
        default:
            break;
    }
    return 0;
}

Result NfpUserInterface::Mount(u64 handle, NfpuDeviceType type, NfpuMountTarget target)
{
    dvstate = NfpuDeviceState_TagMounted;
    return 0;
}

Result NfpUserInterface::Unmount(u64 handle)
{
    dvstate = NfpuDeviceState_SearchingForTag;
    return 0;
}

Result NfpUserInterface::OpenApplicationArea(u64 handle, u32 access_id)
{
    return NfpResults::ResultNotFoundArea;
}

Result NfpUserInterface::GetApplicationArea(u64 handle, OutBuffer<u8> out_area, Out<u32> out_area_size)
{
    out_area_size.SetValue(0);
    return 0;
}

Result NfpUserInterface::SetApplicationArea(u64 handle, InBuffer<u8> area)
{
    return 0;
}

Result NfpUserInterface::Flush(u64 handle)
{
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpUserInterface::Restore(u64 handle)
{
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpUserInterface::CreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area)
{
    return 0;
}

Result NfpUserInterface::GetTagInfo(u64 handle, OutPointerWithServerSize<NfpuTagInfo, 0x1> out_info)
{
    NfpuTagInfo tag_info = AmiiboEmulator::GetCurrentTagInfo();
    if(tag_info.uuid_length == 0) return LibnxError_NotFound;
    memcpy(out_info.pointer, &tag_info, sizeof(NfpuTagInfo));
    return 0;
}

Result NfpUserInterface::GetRegisterInfo(u64 handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info)
{
    NfpuRegisterInfo reg_info = AmiiboEmulator::EmulateRegisterInfo();
    memcpy(out_info.pointer, &reg_info, sizeof(NfpuRegisterInfo));
    return 0;
}

Result NfpUserInterface::GetModelInfo(u64 handle, OutPointerWithServerSize<NfpuModelInfo, 0x1> out_info)
{
    NfpuModelInfo model_info = AmiiboEmulator::GetCurrentModelInfo();
    memcpy(out_info.pointer, &model_info, sizeof(NfpuModelInfo));
    return 0;
}

Result NfpUserInterface::GetCommonInfo(u64 handle, OutPointerWithServerSize<NfpuCommonInfo, 0x1> out_info)
{
    NfpuCommonInfo common_info = AmiiboEmulator::EmulateCommonInfo();
    memcpy(out_info.pointer, &common_info, sizeof(NfpuCommonInfo));
    return 0;
}

Result NfpUserInterface::AttachActivateEvent(u64 handle, Out<CopiedHandle> event)
{
    event.SetValue(g_eactivate->GetHandle());
    return 0;
}

Result NfpUserInterface::AttachDeactivateEvent(u64 handle, Out<CopiedHandle> event)
{
    event.SetValue(edeactivate->GetHandle());
    return 0;
}

Result NfpUserInterface::GetState(Out<u32> out_state)
{
    out_state.SetValue(static_cast<u32>(state));
    return 0;
}

Result NfpUserInterface::GetDeviceState(u64 handle, Out<u32> out_state)
{
    std::scoped_lock<HosMutex> lck(g_comboLock);
    if(g_comboDetected) {
        dvstate = NfpuDeviceState_TagFound;
        g_comboDetected = false;
        g_eactivate->Clear();
    }
    out_state.SetValue(static_cast<u32>(dvstate));
    return 0;
}

Result NfpUserInterface::GetNpadId(u64 handle, Out<u32> out_npad_id)
{
    HidControllerID ctrlid = CONTROLLER_HANDHELD;
    hidScanInput();
    if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) ctrlid = CONTROLLER_PLAYER_1;
    out_npad_id.SetValue(hidControllerIDToOfficial(ctrlid));
    return 0;
}

Result NfpUserInterface::AttachAvailabilityChangeEvent(Out<CopiedHandle> event)
{
    event.SetValue(eavailabilitychange->GetHandle());
    return 0;
}

Result NfpUserInterface::GetApplicationAreaSize(Out<u32> size)
{
    size.SetValue(0);
    return 0;
}

Result NfpUserInterface::RecreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area)
{
    return 0;
}
