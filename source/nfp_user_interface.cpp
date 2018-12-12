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
extern IEvent* g_activate_event;
extern FILE* g_logging_file;

static HosMutex g_event_creation_lock;
static bool g_created_events = false;
static IEvent* g_deactivate_event = nullptr;
static IEvent* g_availability_change_event = nullptr;

struct AmiiboFile {
    std::array<u8, 10> uuid;
    u8 padding[0x4a];
    ModelInfo model_info;
};
static_assert(sizeof(AmiiboFile) == 0x94, "AmiiboFile is an invalid size");

static AmiiboFile GetAmiibo() {
    AmiiboFile amiibo{};

    FILE* file = fopen("amiibo.bin", "rb");
    if (!file) {
        fatalSimple(MAKERESULT(Module_Libnx, 42));
    }
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    fread(&amiibo, fsize, 1, file);
    fclose(file);
    return amiibo;
}

NfpUserInterface::NfpUserInterface(NfpUser *u) {
    fprintf(g_logging_file, "Creating NfpUserInterface\n");
    fflush(g_logging_file);
    this->forward_intf = u;

    {
        std::scoped_lock<HosMutex> lk(g_event_creation_lock);
        if (!g_created_events) {
            g_deactivate_event = CreateWriteOnlySystemEvent();
            g_availability_change_event = CreateWriteOnlySystemEvent();
        }
    }
}

NfpUserInterface::~NfpUserInterface() {
    fprintf(g_logging_file, "Destroying NfpUserInterface\n");
    fflush(g_logging_file);

    nfpuserClose(this->forward_intf);
    delete this->forward_intf;
}

Result NfpUserInterface::Initialize(u64 aruid, PidDescriptor pid_desc, InBuffer<u8> buf) {
    fprintf(g_logging_file, "Calling initialize(0x%016lx, 0x%016lx)\n", aruid, pid_desc.pid);
    fflush(g_logging_file);

    state = State::Initialized;
    device_state = DeviceState::Initialized;
    return 0;
}

Result NfpUserInterface::Finalize() {
    fprintf(g_logging_file, "Calling finalize\n");
    fflush(g_logging_file);
    state = State::NonInitialized;
    device_state = DeviceState::Finalized;
    return 0;
}

Result NfpUserInterface::ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u32> out_count) {
    fprintf(g_logging_file, "In ListDevices\n");
    fflush(g_logging_file);
    if (out_devices.num_elements >= 1) {
        memcpy(out_devices.pointer, &device_handle, sizeof(device_handle));
        out_count.SetValue(1);
    } else {
        out_count.SetValue(0);
    }
    return 0;
}

Result NfpUserInterface::StartDetection(u64 handle) {
    fprintf(g_logging_file, "In StartDetection\n");
    fflush(g_logging_file);
    if (device_state == DeviceState::Initialized || device_state == DeviceState::TagRemoved) {
        device_state = DeviceState::SearchingForTag;
    }
    return 0;
}

Result NfpUserInterface::StopDetection(u64 handle) {
    fprintf(g_logging_file, "In StopDetection\n");
    fflush(g_logging_file);
    switch (device_state) {
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

Result NfpUserInterface::Mount(u64 handle, u32 type, u32 target) {
    fprintf(g_logging_file, "In Mount\n");
    fflush(g_logging_file);
    device_state = DeviceState::TagNearby;
    return 0;
}

Result NfpUserInterface::Unmount(u64 handle) {
    fprintf(g_logging_file, "In Unmount\n");
    fflush(g_logging_file);
    device_state = DeviceState::TagFound;
    return 0;
}

Result NfpUserInterface::OpenApplicationArea(u64 handle, u32 access_id) {
    fprintf(g_logging_file, "In OpenApplicationArea\n");
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::GetApplicationArea(u64 handle, OutBuffer<u8> out_area, Out<u32> out_area_size) {
    fprintf(g_logging_file, "In GetApplicationArea\n");
    fflush(g_logging_file);
    out_area_size.SetValue(0);
    return 0;
}

Result NfpUserInterface::SetApplicationArea(u64 handle, InBuffer<u8> area) {
    fprintf(g_logging_file, "In SetApplicationArea\n");
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::Flush(u64 handle) {
    fprintf(g_logging_file, "In Flush\n");
    fflush(g_logging_file);
    device_state = DeviceState::TagFound;
    return 0;
}

Result NfpUserInterface::Restore(u64 handle) {
    fprintf(g_logging_file, "In Restore\n");
    fflush(g_logging_file);
    device_state = DeviceState::TagFound;
    return 0;
}

Result NfpUserInterface::CreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area) {
    fprintf(g_logging_file, "In CreateApplicationArea\n");
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::GetTagInfo(OutPointerWithServerSize<TagInfo, 0x1> out_info) {
    fprintf(g_logging_file, "In GetTagInfo\n");
    fflush(g_logging_file);
    
    auto amiibo = GetAmiibo();

    TagInfo tag_info{};
    tag_info.uuid = amiibo.uuid;
    tag_info.uuid_length = static_cast<u8>(tag_info.uuid.size());

    tag_info.protocol = 1; // TODO(ogniK): Figure out actual values
    tag_info.tag_type = 2;

    *out_info.pointer = tag_info;
    return 0;
}

Result NfpUserInterface::GetRegisterInfo(OutPointerWithServerSize<RegisterInfo, 0x1> out_info) {
    fprintf(g_logging_file, "In GetRegisterInfo\n");
    fflush(g_logging_file);
    return 0;
}

Result NfpUserInterface::GetModelInfo(OutPointerWithServerSize<ModelInfo, 0x1> out_info) {
    fprintf(g_logging_file, "In GetModelInfo\n");
    fflush(g_logging_file);
    auto amiibo = GetAmiibo();
    
    *out_info.pointer = amiibo.model_info;
    return 0;
}

Result NfpUserInterface::GetCommonInfo(OutPointerWithServerSize<CommonInfo, 0x1> out_info) {
    fprintf(g_logging_file, "In GetCommonInfo\n");
    fflush(g_logging_file);
    CommonInfo common_info{};
    common_info.application_area_size = 0;
    
    *out_info.pointer = common_info;
    return 0;
}

Result NfpUserInterface::AttachActivateEvent(u64 handle, Out<CopiedHandle> event) {
    fprintf(g_logging_file, "In AttachActivateEvent\n");
    fflush(g_logging_file);
    event.SetValue(g_activate_event->GetHandle());
    has_attached_handle = true;
    return 0;
}

Result NfpUserInterface::AttachDeactivateEvent(u64 handle, Out<CopiedHandle> event) {
    fprintf(g_logging_file, "In AttachDeactivateEvent\n");
    fflush(g_logging_file);
    event.SetValue(g_deactivate_event->GetHandle());
    return 0;
}

Result NfpUserInterface::GetState(Out<u32> out_state) {
    fprintf(g_logging_file, "In GetState\n");
    fflush(g_logging_file);
    out_state.SetValue(static_cast<u32>(state));
    return 0;
}

Result NfpUserInterface::GetDeviceState(u64 handle, Out<u32> out_state) {
    fprintf(g_logging_file, "In GetDeviceState\n");
    fflush(g_logging_file);
    if (g_key_combo_triggered && !has_attached_handle) {
        fprintf(g_logging_file, "Triggered GetDeviceState\n");
        fflush(g_logging_file);

        device_state = DeviceState::TagFound;
        g_key_combo_triggered = false;
        g_activate_event->Clear();
    }

    out_state.SetValue(static_cast<u32>(device_state));
    return 0;
}

Result NfpUserInterface::GetNpadId(u64 handle, Out<u32> out_npad_id) {
    fprintf(g_logging_file, "In GetNpadId\n");
    fflush(g_logging_file);
    out_npad_id.SetValue(npad_id);
    return 0;
}

Result NfpUserInterface::AttachAvailabilityChangeEvent(Out<CopiedHandle> event) {
    fprintf(g_logging_file, "In AttachAvailabilityChangeEvent\n");
    fflush(g_logging_file);
    event.SetValue(g_availability_change_event->GetHandle());
    return 0;
}

Result NfpUserInterface::GetApplicationAreaSize(Out<u32> size) {
    fprintf(g_logging_file, "In GetApplicationAreaSize\n");
    fflush(g_logging_file);
    size.SetValue(0);
    return 0;
}

Result NfpUserInterface::RecreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area) {
    fprintf(g_logging_file, "In RecreateApplicationArea\n");
    fflush(g_logging_file);
    return 0;
}
