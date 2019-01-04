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

extern std::atomic_bool g_key_combo_triggered;
extern IEvent *g_activate_event;
extern FILE *g_logging_file;

static HosMutex g_event_creation_lock;
static HosMutex g_device_state_lock;
static HosMutex g_state_lock;
static bool g_created_events = false;
static IEvent *g_deactivate_event = nullptr;
static IEvent *g_availability_change_event = nullptr;

struct AmiiboFile
{
    u8 uuid[10];
    u8 padding[0x4a];
    ModelInfo model_info;
};
static_assert(sizeof(AmiiboFile) == 0x94, "AmiiboFile is an invalid size");


// https://gist.github.com/ccbrown/9722406
static void DumpHex(const void *data, size_t size)
{
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i)
    {
        fprintf(g_logging_file, "%02X ", ((unsigned char *)data)[i]);
        if (((unsigned char *)data)[i] >= ' ' && ((unsigned char *)data)[i] <= '~')
        {
            ascii[i % 16] = ((unsigned char *)data)[i];
        }
        else
        {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size)
        {
            fprintf(g_logging_file, " ");
            if ((i + 1) % 16 == 0)
            {
                fprintf(g_logging_file, "|  %s \n", ascii);
            }
            else if (i + 1 == size)
            {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8)
                {
                    fprintf(g_logging_file, " ");
                }
                for (j = (i + 1) % 16; j < 16; ++j)
                {
                    fprintf(g_logging_file, "   ");
                }
                fprintf(g_logging_file, "|  %s \n", ascii);
            }
        }
    }
    fflush(g_logging_file);
}

void NfpUserInterface::SetDeviceState(DeviceState _state) {
    std::scoped_lock<HosMutex> lk(g_device_state_lock);
    device_state = _state;
}

void NfpUserInterface::SetState(State _state) {
    std::scoped_lock<HosMutex> lk(g_state_lock);
    state = _state;
}

static AmiiboFile GetAmiibo()
{
    fprintf(g_logging_file, "GetAmiibo(AmiiboFile[0x1] {\n");
    fflush(g_logging_file);
    AmiiboFile amiibo{};

    FILE *file = fopen("amiibo.bin", "rb");
    if (!file)
    {
        fatalSimple(MAKERESULT(Module_Libnx, 42));
    }
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(fsize < sizeof(AmiiboFile)) {
        return amiibo;
    }


    fread(&amiibo, sizeof(AmiiboFile), 1, file);
    fclose(file);

    // DumpHex(&amiibo, fsize);
    // fprintf(g_logging_file, "})\n");
    // fflush(g_logging_file);

    return amiibo;
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
    DumpHex(buf.buffer, buf.num_elements);
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
        SetDeviceState(DeviceState::SearchingForTag);
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
    }
    return 0;
}

Result NfpUserInterface::Mount(u64 handle, u32 type, u32 target)
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
    device_state = DeviceState::TagFound;
    return 0;
}

Result NfpUserInterface::OpenApplicationArea(u64 handle, u32 access_id)
{
    fprintf(g_logging_file, "NfpUserInterface::OpenApplicationArea(0x%016lx, 0x%08x)\n", handle, access_id);
    fflush(g_logging_file);
    return 0;
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

Result NfpUserInterface::GetTagInfo(OutPointerWithServerSize<TagInfo, 0x1> out_info)
{
    fprintf(g_logging_file, "NfpUserInterface::GetTagInfo(TagInfo[0x%lx])\n", out_info.num_elements);
    fflush(g_logging_file);

    auto amiibo = GetAmiibo();

    TagInfo tag_info = {0};
    memcpy(&tag_info.uuid[0], &amiibo.uuid[0], 3);
    memcpy(&tag_info.uuid[3], &amiibo.uuid[4], 4);
    tag_info.uuid_length = static_cast<u8>(7);

    tag_info.protocol = 1; // TODO(ogniK): Figure out actual values
    tag_info.tag_type = 2;

    *out_info.pointer = tag_info;
    fprintf(g_logging_file, "NfpUserInterface::EndOfTagInfo(tag_info[0x%lx]{\n", sizeof(TagInfo));
    fflush(g_logging_file);
    DumpHex(&tag_info, sizeof(TagInfo));
    fprintf(g_logging_file, "})\n");
    fflush(g_logging_file);
    
    return 0;
}

Result NfpUserInterface::GetRegisterInfo(OutPointerWithServerSize<RegisterInfo, 0x1> out_info)
{
    fprintf(g_logging_file, "NfpUserInterface::GetRegisterInfo(RegisterInfo[0x%lx])\n", out_info.num_elements);
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::GetModelInfo(OutPointerWithServerSize<ModelInfo, 0x1> out_info)
{
    fprintf(g_logging_file, "NfpUserInterface::GetModelInfo(ModelInfo[0x%lx])\n", out_info.num_elements);
    fflush(g_logging_file);

    auto amiibo = GetAmiibo();

    ModelInfo model_info = {0};
    memcpy(&model_info.amiibo_identification_block[0], &amiibo.model_info.amiibo_identification_block[0], 3);
    model_info.amiibo_identification_block[3] = amiibo.model_info.amiibo_identification_block[6];
    model_info.amiibo_identification_block[4] = amiibo.model_info.amiibo_identification_block[5];
    model_info.amiibo_identification_block[5] = amiibo.model_info.amiibo_identification_block[4];

    *out_info.pointer = model_info;
    fprintf(g_logging_file, "NfpUserInterface::EndOfModelInfo(model_info[0x%lx]{\n", sizeof(ModelInfo));
    fflush(g_logging_file);
    DumpHex(&model_info, sizeof(ModelInfo));
    fprintf(g_logging_file, "})\n");
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::GetCommonInfo(OutPointerWithServerSize<CommonInfo, 0x1> out_info)
{
    fprintf(g_logging_file, "NfpUserInterface::GetCommonInfo(CommonInfo[0x%lx])\n", out_info.num_elements);
    fflush(g_logging_file);
    CommonInfo common_info{};
    common_info.application_area_size = 0;

    *out_info.pointer = common_info;
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
    fprintf(g_logging_file, "NfpUserInterface::GetDeviceState(0x%016lx), current state is 0x%x\n", handle, static_cast<u32>(device_state));
    fflush(g_logging_file);
    if (g_key_combo_triggered && has_attached_handle)
    {
        fprintf(g_logging_file, "Triggered GetDeviceState\n");
        fflush(g_logging_file);

        device_state = DeviceState::TagFound;
        g_key_combo_triggered = false;
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
    DumpHex(area.buffer, area.num_elements);
    fprintf(g_logging_file, "})\n");
    fflush(g_logging_file);

    return 0;
}
