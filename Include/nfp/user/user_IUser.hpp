 
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <nfp/nfp_Types.hpp>

namespace nfp::user
{
    class IUser final : public IServiceObject
    {
        private:

            NfpuState state;
            NfpuDeviceState deviceState;
            IEvent *eventActivate;
            IEvent *eventDeactivate;
            IEvent *eventAvailabilityChange;
            u32 currentAreaAppId;

        public:

            IUser();
            ~IUser();

        private:

            enum class CommandId
            {
                Initialize = 0,
                Finalize = 1,
                ListDevices = 2,
                StartDetection = 3,
                StopDetection = 4,
                Mount = 5,
                Unmount = 6,
                OpenApplicationArea = 7,
                GetApplicationArea = 8,
                SetApplicationArea = 9,
                Flush = 10,
                Restore = 11,
                CreateApplicationArea = 12,
                GetTagInfo = 13,
                GetRegisterInfo = 14,
                GetCommonInfo = 15,
                GetModelInfo = 16,
                AttachActivateEvent = 17,
                AttachDeactivateEvent = 18,
                GetState = 19,
                GetDeviceState = 20,
                GetNpadId = 21,
                GetApplicationAreaSize = 22,
                AttachAvailabilityChangeEvent = 23,
                RecreateApplicationArea = 24,
            };

            Result Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf);
            Result Finalize();
            Result ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u32> out_count);
            Result StartDetection(DeviceHandle handle);
            Result StopDetection(DeviceHandle handle);
            Result Mount(DeviceHandle handle, u32 type, u32 target);
            Result Unmount(DeviceHandle handle);
            Result OpenApplicationArea(Out<u32> npad_id, DeviceHandle handle, u32 id);
            Result GetApplicationArea(OutBuffer<u8> data, Out<u32> data_size, DeviceHandle handle);
            Result SetApplicationArea(InBuffer<u8> data, DeviceHandle handle);
            Result Flush(DeviceHandle handle);
            Result Restore(DeviceHandle handle);
            Result CreateApplicationArea(InBuffer<u8> data, DeviceHandle handle, u32 id);
            Result GetTagInfo(DeviceHandle handle, OutPointerWithServerSize<TagInfo, 1> out_info);
            Result GetRegisterInfo(DeviceHandle handle, OutPointerWithServerSize<RegisterInfo, 1> out_info);
            Result GetCommonInfo(DeviceHandle handle, OutPointerWithServerSize<CommonInfo, 1> out_info);
            Result GetModelInfo(DeviceHandle handle, OutPointerWithServerSize<ModelInfo, 1> out_info);
            Result AttachActivateEvent(DeviceHandle handle, Out<CopiedHandle> event);
            Result AttachDeactivateEvent(DeviceHandle handle, Out<CopiedHandle> event);
            Result GetState(Out<u32> state);
            Result GetDeviceState(DeviceHandle handle, Out<u32> state);
            Result GetNpadId(DeviceHandle handle, Out<u32> npad_id);
            Result GetApplicationAreaSize(DeviceHandle handle, Out<u32> size);
            Result AttachAvailabilityChangeEvent(Out<CopiedHandle> event);
            Result RecreateApplicationArea(InBuffer<u8> data, DeviceHandle handle, u32 id);

        public:
        
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(IUser, Initialize),
                MAKE_SERVICE_COMMAND_META(IUser, Finalize),
                MAKE_SERVICE_COMMAND_META(IUser, ListDevices),
                MAKE_SERVICE_COMMAND_META(IUser, StartDetection),
                MAKE_SERVICE_COMMAND_META(IUser, StopDetection),
                MAKE_SERVICE_COMMAND_META(IUser, Mount),
                MAKE_SERVICE_COMMAND_META(IUser, Unmount),
                MAKE_SERVICE_COMMAND_META(IUser, OpenApplicationArea),
                MAKE_SERVICE_COMMAND_META(IUser, GetApplicationArea),
                MAKE_SERVICE_COMMAND_META(IUser, SetApplicationArea),
                MAKE_SERVICE_COMMAND_META(IUser, Flush),
                MAKE_SERVICE_COMMAND_META(IUser, Restore),
                MAKE_SERVICE_COMMAND_META(IUser, CreateApplicationArea),
                MAKE_SERVICE_COMMAND_META(IUser, GetTagInfo),
                MAKE_SERVICE_COMMAND_META(IUser, GetRegisterInfo),
                MAKE_SERVICE_COMMAND_META(IUser, GetCommonInfo),
                MAKE_SERVICE_COMMAND_META(IUser, GetModelInfo),
                MAKE_SERVICE_COMMAND_META(IUser, AttachActivateEvent),
                MAKE_SERVICE_COMMAND_META(IUser, AttachDeactivateEvent),
                MAKE_SERVICE_COMMAND_META(IUser, GetState),
                MAKE_SERVICE_COMMAND_META(IUser, GetDeviceState),
                MAKE_SERVICE_COMMAND_META(IUser, GetNpadId),
                MAKE_SERVICE_COMMAND_META(IUser, AttachAvailabilityChangeEvent),
                MAKE_SERVICE_COMMAND_META(IUser, GetApplicationAreaSize),
                MAKE_SERVICE_COMMAND_META(IUser, RecreateApplicationArea),
            };
    };

    void CreateCommonActivateEvent();
}