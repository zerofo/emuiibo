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

extern IEvent *g_eactivate;
extern FILE *g_logging_file;
extern u32 g_toggleEmulation;

HosMutex g_toggleLock;

NfpUserInterface::NfpUserInterface(NfpUser *u)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    this->forward_intf = u;
    edeactivate = CreateWriteOnlySystemEvent();
    eavailabilitychange = CreateWriteOnlySystemEvent();
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Unavailable;
}

NfpUserInterface::~NfpUserInterface()
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    delete edeactivate;
    delete eavailabilitychange;
    nfpUserClose(this->forward_intf);
    delete this->forward_intf;
}

Result NfpUserInterface::Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    state = NfpuState_Initialized;
    dvstate = NfpuDeviceState_Initialized;
    return 0;
}

Result NfpUserInterface::Finalize()
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Finalized;
    return 0;
}

Result NfpUserInterface::ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u64> out_count)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    u64 dvcid = 0x20;
    hidScanInput();
    if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) dvcid = (u64)CONTROLLER_PLAYER_1;
    memcpy(out_devices.pointer, &dvcid, sizeof(u64));
    out_count.SetValue(1);
    return 0;
}

Result NfpUserInterface::StartDetection(u64 handle)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    g_eactivate->Signal();
    eavailabilitychange->Signal();
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpUserInterface::StopDetection(u64 handle)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    switch(dvstate)
    {
        case NfpuDeviceState_TagFound:
        case NfpuDeviceState_TagMounted:
            edeactivate->Signal();
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
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    g_eactivate->Signal();
    dvstate = NfpuDeviceState_TagMounted;
    return 0;
}

Result NfpUserInterface::Unmount(u64 handle)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    edeactivate->Signal();
    dvstate = NfpuDeviceState_SearchingForTag;
    return 0;
}

Result NfpUserInterface::OpenApplicationArea(u64 handle, u32 access_id)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    return NfpResults::ResultNotFoundArea;
}

Result NfpUserInterface::GetApplicationArea(u64 handle, OutBuffer<u8> out_area, Out<u32> out_area_size)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    out_area_size.SetValue(out_area.num_elements);
    return 0;
}

Result NfpUserInterface::SetApplicationArea(u64 handle, InBuffer<u8> area)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::Flush(u64 handle)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpUserInterface::Restore(u64 handle)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpUserInterface::CreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::GetTagInfo(u64 handle, OutPointerWithServerSize<NfpuTagInfo, 0x1> out_info)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    NfpuTagInfo tag_info = AmiiboEmulator::GetCurrentTagInfo();
    if(tag_info.uuid_length == 0) return LibnxError_NotFound;
    memcpy(out_info.pointer, &tag_info, sizeof(NfpuTagInfo));
    return 0;
}

Result NfpUserInterface::GetRegisterInfo(u64 handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    NfpuRegisterInfo reg_info = AmiiboEmulator::EmulateRegisterInfo();
    memcpy(out_info.pointer, &reg_info, sizeof(NfpuRegisterInfo));
    return 0;
}

Result NfpUserInterface::GetModelInfo(u64 handle, OutPointerWithServerSize<NfpuModelInfo, 0x1> out_info)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    NfpuModelInfo model_info = AmiiboEmulator::GetCurrentModelInfo();
    memcpy(out_info.pointer, &model_info, sizeof(NfpuModelInfo));
    return 0;
}

Result NfpUserInterface::GetCommonInfo(u64 handle, OutPointerWithServerSize<NfpuCommonInfo, 0x1> out_info)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    NfpuCommonInfo common_info = AmiiboEmulator::EmulateCommonInfo();
    memcpy(out_info.pointer, &common_info, sizeof(NfpuCommonInfo));
    return 0;
}

Result NfpUserInterface::AttachActivateEvent(u64 handle, Out<CopiedHandle> event)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    event.SetValue(g_eactivate->GetHandle());
    return 0;
}

Result NfpUserInterface::AttachDeactivateEvent(u64 handle, Out<CopiedHandle> event)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    event.SetValue(edeactivate->GetHandle());
    return 0;
}

Result NfpUserInterface::GetState(Out<u32> out_state)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    out_state.SetValue(static_cast<u32>(state));
    return 0;
}

Result NfpUserInterface::GetDeviceState(u64 handle, Out<u32> out_state)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    out_state.SetValue(static_cast<u32>(dvstate));
    return 0;
}

Result NfpUserInterface::GetNpadId(u64 handle, Out<u32> out_npad_id)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    out_npad_id.SetValue((u32)handle);
    return 0;
}

Result NfpUserInterface::AttachAvailabilityChangeEvent(Out<CopiedHandle> event)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    event.SetValue(eavailabilitychange->GetHandle());
    return 0;
}

Result NfpUserInterface::GetApplicationAreaSize(Out<u32> size)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    size.SetValue(0);
    return 0;
}

Result NfpUserInterface::RecreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area)
{
    fprintf(g_logging_file, " -- IUser-%s(...)\n", __FUNCTION__);
    fflush(g_logging_file);
    return 0;
}