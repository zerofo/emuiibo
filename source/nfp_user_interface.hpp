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

struct TagInfo {
    std::array<u8, 10> uuid;
    u8 uuid_length; // TODO(ogniK): Figure out if this is actual the uuid length or does it
                    // mean something else
    u8 pad1[0x15];
    u32 protocol;
    u32 tag_type;
    u8 pad2[0x2c];
};
static_assert(sizeof(TagInfo) == 0x54, "TagInfo is an invalid size");

struct RegisterInfo {
    std::array<u8, 0x100> data; /* TODO: Struct layout */
};
static_assert(sizeof(RegisterInfo) == 0x100, "RegisterInfo is an invalid size");

struct CommonInfo {
    u16 last_write_year; // be
    u8 last_write_month;
    u8 last_write_day;
    u16 write_counter; // be
    u16 version; // be
    u32 application_area_size; // be
    u8 padding[0x34];
};
static_assert(sizeof(CommonInfo) == 0x40, "CommonInfo is an invalid size");

struct ModelInfo {
    std::array<u8, 0x8> amiibo_identification_block;
    u8 padding[0x38];
};
static_assert(sizeof(ModelInfo) == 0x40, "ModelInfo is an invalid size");

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

class NfpUserInterface : public IServiceObject {
    private:
        NfpUser *forward_intf;
    public:
        NfpUserInterface(NfpUser *u);
        ~NfpUserInterface();
        
    private:
        /* Actual command API. */
        virtual Result Initialize(u64 aruid, PidDescriptor pid_desc, InBuffer<u8> buf) final;
        virtual Result Finalize() final;
        virtual Result ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u64> out_count) final;
        virtual Result StartDetection(u64 handle) final;
        virtual Result StopDetection(u64 handle) final;
        virtual Result Mount(u64 handle, u32 type, u32 target) final;
        virtual Result Unmount(u64 handle) final;
        virtual Result OpenApplicationArea(u64 handle, u32 access_id) final;
        virtual Result GetApplicationArea(u64 handle, OutBuffer<u8> out_area, Out<u32> out_area_size) final;
        virtual Result SetApplicationArea(u64 handle, InBuffer<u8> area) final;
        virtual Result Flush(u64 handle) final;
        virtual Result Restore(u64 handle) final;
        virtual Result CreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area) final;
        virtual Result GetTagInfo(OutPointerWithServerSize<TagInfo, 0x1> out_info) final;
        virtual Result GetRegisterInfo(OutPointerWithServerSize<RegisterInfo, 0x1> out_info) final;
        virtual Result GetCommonInfo(OutPointerWithServerSize<CommonInfo, 0x1> out_info) final;
        virtual Result GetModelInfo(OutPointerWithServerSize<ModelInfo, 0x1> out_info) final;
        virtual Result AttachActivateEvent(u64 handle, Out<CopiedHandle> event) final;
        virtual Result AttachDeactivateEvent(u64 handle, Out<CopiedHandle> event) final;
        virtual Result GetState(Out<u32> state) final;
        virtual Result GetDeviceState(u64 handle, Out<u32> state) final;
        virtual Result GetNpadId(u64 handle, Out<u32> npad_id) final;
        virtual Result AttachAvailabilityChangeEvent(Out<CopiedHandle> event) final;
        virtual Result GetApplicationAreaSize(Out<u32> size) final;
        virtual Result RecreateApplicationArea(u64 handle, u32 access_id, InBuffer<u8> area) final;

        bool has_attached_handle{};
        const u64 device_handle{0xcafebabe}; // 'YUZU'
        const u32 npad_id{0x20}; // Player 1 controller
        State state{State::NonInitialized};
        DeviceState device_state{DeviceState::Initialized};
        IEvent* deactivate_event;
        IEvent* availability_change_event;

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
