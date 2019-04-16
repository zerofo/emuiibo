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

extern std::atomic_bool g_enableComboSet;
extern IEvent *g_activate_event;
extern FILE *g_logging_file;

static HosMutex g_event_creation_lock;
static HosMutex g_device_state_lock;
static HosMutex g_state_lock;
HosMutex g_enable_lock;
static bool g_created_events = false;
static IEvent *g_deactivate_event = nullptr;
static IEvent *g_availability_change_event = nullptr;

void NfpUserInterface::SetDeviceState(DeviceState _state) {
    std::scoped_lock<HosMutex> lk(g_device_state_lock);
    device_state = _state;
}

void NfpUserInterface::SetState(State _state) {
    std::scoped_lock<HosMutex> lk(g_state_lock);
    state = _state;
}

NfpUserInterface::NfpUserInterface(NfpUser *u)
{
    fprintf(g_logging_file, "NfpUserInterface::NfpUserInterface()\n");
    fflush(g_logging_file);
    this->forward_intf = u;

    {
        std::scoped_lock<HosMutex> lk(g_event_creation_lock);
        if (!g_created_events)
        {
            g_deactivate_event = CreateWriteOnlySystemEvent();
            g_availability_change_event = CreateWriteOnlySystemEvent();
        }
    }
}

NfpUserInterface::~NfpUserInterface()
{
    fprintf(g_logging_file, "NfpUserInterface::~NfpUserInterface()\n");
    fflush(g_logging_file);

    nfpuserClose(this->forward_intf);
    delete this->forward_intf;
}

Result NfpUserInterface::Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf)
{
    fprintf(g_logging_file, "NfpUserInterface::Initialize(0x%016lx, 0x%016lx, pid (%016lx), buf[0x%lx] {\n", aruid, unk, pid_desc.pid, buf.num_elements);
    fflush(g_logging_file);
    fprintf(g_logging_file, "});\n");
    fflush(g_logging_file);

    SetDeviceState(DeviceState::Initialized);
    SetState(State::Initialized);

    return 0;
}

Result NfpUserInterface::Finalize()
{
    fprintf(g_logging_file, "NfpUserInterface::Finalize();\n");
    fflush(g_logging_file);
    state = State::NonInitialized;
    device_state = DeviceState::Finalized;
    return 0;
}

Result NfpUserInterface::ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u64> out_count)
{
    fprintf(g_logging_file, "NfpUserInterface::ListDevices(u64[0x%lx]);\n", out_devices.num_elements);
    fflush(g_logging_file);
    if (out_devices.num_elements >= 1)
    {
        memcpy(out_devices.pointer, &device_handle, sizeof(device_handle));
        out_count.SetValue(1);
    }
    else
    {
        fprintf(g_logging_file, "elements are zero\n");
        fflush(g_logging_file);
        out_count.SetValue(0);
    }
    return 0;
}

Result NfpUserInterface::StartDetection(u64 handle)
{
    fprintf(g_logging_file, "NfpUserInterface::StartDetection(0x%016lx);\n", handle);
    fflush(g_logging_file);
    if (device_state == DeviceState::Initialized || device_state == DeviceState::TagRemoved)
    {
        std::scoped_lock<HosMutex> lck(g_enable_lock);
        if(g_enableComboSet) {
            g_availability_change_event->Signal();
            g_enableComboSet = false;
        }
        SetDeviceState(DeviceState::TagFound);
        return 0;
    } else {
        fprintf(g_logging_file, "Bad State\n");
        fflush(g_logging_file);
        return 0xdead;
    }
}

Result NfpUserInterface::StopDetection(u64 handle)
{
    fprintf(g_logging_file, "NfpUserInterface::StopDetection(0x%016lx)\n", handle);
    fflush(g_logging_file);
    switch (device_state)
    {
    case DeviceState::TagFound:
    case DeviceState::TagNearby:
        g_deactivate_event->Signal();
        device_state = DeviceState::Initialized;
        break;
    case DeviceState::SearchingForTag:
    case DeviceState::TagRemoved:
        device_state = DeviceState::Initialized;
        break;
    default:
        break;
    }
    return 0;
}

Result NfpUserInterface::Mount(u64 handle, NfpuDeviceType type, NfpuMountTarget target)
{
    fprintf(g_logging_file, "NfpUserInterface::Mount(0x%016lx, 0x%08x, 0x%08x)\n", handle, type, target);
    fflush(g_logging_file);
    device_state = DeviceState::TagNearby;
    return 0;
}

Result NfpUserInterface::Unmount(u64 handle)
{
    fprintf(g_logging_file, "NfpUserInterface::Unmount(0x%016lx)\n", handle);
    fflush(g_logging_file);
    device_state = DeviceState::SearchingForTag;
    return 0;
}

Result NfpUserInterface::OpenApplicationArea(u64 handle, u32 access_id)
{
    fprintf(g_logging_file, "NfpUserInterface::OpenApplicationArea(0x%016lx, 0x%08x)\n", handle, access_id);
    fflush(g_logging_file);
    return 0x10073;
}

Result NfpUserInterface::GetApplicationArea(u64 handle, OutBuffer<u8> out_area, Out<u32> out_area_size)
{
    fprintf(g_logging_file, "NfpUserInterface::GetApplicationArea(0x%016lx)\n", handle);
    fflush(g_logging_file);
    out_area_size.SetValue(0);
    return 0;
}

Result NfpUserInterface::SetApplicationArea(u64 handle, InBuffer<u8> area)
{
    fprintf(g_logging_file, "NfpUserInterface::SetApplicationArea(0x%016lx, u8[0x%lx])\n", handle, area.num_elements);
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::Flush(u64 handle)
{
    fprintf(g_logging_file, "NfpUserInterface::Flush(0x%016lx)\n", handle);
    fflush(g_logging_file);
    device_state = DeviceState::TagFound;
    return 0;
}

Result NfpUserInterface::Restore(u64 handle)
{
    fprintf(g_logging_file, "NfpUserInterface::Restore(0x%016lx)\n", handle);
    fflush(g_logging_file);
    device_state = DeviceState::TagFound;
    return 0;
}

Result NfpUserInterface::CreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area)
{
    fprintf(g_logging_file, "NfpUserInterface::CreateApplicationArea(0x%016lx, 0x%08x)\n", handle, access_id);
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::GetTagInfo(u64 handle, OutPointerWithServerSize<NfpuTagInfo, 0x1> out_info)
{
    fprintf(g_logging_file, "NfpUserInterface::GetTagInfo(TagInfo[0x%lx])\n", out_info.num_elements);
    fflush(g_logging_file);
    
    NfpuTagInfo tag_info = AmiiboEmulator::GetCurrentTagInfo();
    if(tag_info.uuid_length == 0) return LibnxError_NotFound;

    memcpy(out_info.pointer, &tag_info, sizeof(NfpuTagInfo));
    return 0;
}

Result NfpUserInterface::GetRegisterInfo(u64 handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info)
{
    fprintf(g_logging_file, "NfpUserInterface::GetRegisterInfo(RegisterInfo[0x%lx])\n", out_info.num_elements);
    fflush(g_logging_file);

    NfpuRegisterInfo reg_info = AmiiboEmulator::EmulateRegisterInfo();
    memcpy(out_info.pointer, &reg_info, sizeof(NfpuRegisterInfo));
    return 0;
}

Result NfpUserInterface::GetModelInfo(u64 handle, OutPointerWithServerSize<NfpuModelInfo, 0x1> out_info)
{
    fprintf(g_logging_file, "NfpUserInterface::GetModelInfo(ModelInfo[0x%lx])\n", out_info.num_elements);
    fflush(g_logging_file);

    NfpuModelInfo model_info = AmiiboEmulator::GetCurrentModelInfo();
    memcpy(out_info.pointer, &model_info, sizeof(NfpuModelInfo));
    return 0;
}

Result NfpUserInterface::GetCommonInfo(u64 handle, OutPointerWithServerSize<NfpuCommonInfo, 0x1> out_info)
{
    fprintf(g_logging_file, "NfpUserInterface::GetCommonInfo(CommonInfo[0x%lx])\n", out_info.num_elements);
    fflush(g_logging_file);

    NfpuCommonInfo common_info = AmiiboEmulator::EmulateCommonInfo();
    memcpy(out_info.pointer, &common_info, sizeof(NfpuCommonInfo));
    return 0;
}

Result NfpUserInterface::AttachActivateEvent(u64 handle, Out<CopiedHandle> event)
{
    fprintf(g_logging_file, "NfpUserInterface::AttachActivateEvent(0x%016lx)\n", handle);
    fflush(g_logging_file);
    event.SetValue(g_activate_event->GetHandle());
    has_attached_handle = true;
    return 0;
}

Result NfpUserInterface::AttachDeactivateEvent(u64 handle, Out<CopiedHandle> event)
{
    fprintf(g_logging_file, "NfpUserInterface::AttachDeactivateEvent(0x%016lx)\n", handle);
    fflush(g_logging_file);
    event.SetValue(g_deactivate_event->GetHandle());
    return 0;
}

Result NfpUserInterface::GetState(Out<u32> out_state)
{
    fprintf(g_logging_file, "NfpUserInterface::GetState(), current state is 0x%x\n", static_cast<u32>(state));
    fflush(g_logging_file);
    out_state.SetValue(static_cast<u32>(state));
    return 0;
}

Result NfpUserInterface::GetDeviceState(u64 handle, Out<u32> out_state)
{
    std::scoped_lock<HosMutex> lck(g_enable_lock);
    fprintf(g_logging_file, "NfpUserInterface::GetDeviceState(0x%016lx), current state is 0x%x\n", handle, static_cast<u32>(device_state));
    fflush(g_logging_file);
    if (g_enableComboSet)// && has_attached_handle)
    {
        fprintf(g_logging_file, "Triggered GetDeviceState\n");
        fflush(g_logging_file);

        device_state = DeviceState::TagFound;
        g_enableComboSet = false;
        g_activate_event->Clear();
    }

    out_state.SetValue(static_cast<u32>(device_state));
    return 0;
}

Result NfpUserInterface::GetNpadId(u64 handle, Out<u32> out_npad_id)
{
    fprintf(g_logging_file, "NfpUserInterface::GetNpadId(0x%016lx)\n", handle);
    fflush(g_logging_file);
    out_npad_id.SetValue(npad_id);
    return 0;
}

Result NfpUserInterface::AttachAvailabilityChangeEvent(Out<CopiedHandle> event)
{
    fprintf(g_logging_file, "NfpUserInterface::AttachAvailabilityChangeEvent()\n");
    fflush(g_logging_file);
    event.SetValue(g_availability_change_event->GetHandle());
    return 0;
}

Result NfpUserInterface::GetApplicationAreaSize(Out<u32> size)
{
    fprintf(g_logging_file, "NfpUserInterface::AttachAvailabilityChangeEvent()\n");
    fflush(g_logging_file);
    size.SetValue(0);
    return 0;
}

Result NfpUserInterface::RecreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area)
{
    fprintf(g_logging_file, "NfpUserInterface::AttachAvailabilityChangeEvent(0x%016lx, 0x%08x, u8[0x%lx]{\n", handle, access_id, area.num_elements);
    fflush(g_logging_file);
    fprintf(g_logging_file, "})\n");
    fflush(g_logging_file);

    return 0;
}
