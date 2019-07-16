 
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <nfp/nfp_Types.hpp>

namespace nfp::sys
{
    class ISystem final : public IServiceObject
    {
        private:

            NfpuState state;
            NfpuDeviceState deviceState;
            IEvent *eventActivate;
            IEvent *eventDeactivate;
            IEvent *eventAvailabilityChange;
            u32 currentAreaAppId;

        public:

            ISystem();
            ~ISystem();

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

                Format = 100,
                GetAdminInfo = 101,
                GetRegisterInfo2 = 102,
                SetRegisterInfo = 103,
                DeleteRegisterInfo = 104,
                DeleteApplicationArea = 105,
                ExistsApplicationArea = 106,
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

            Result Format(DeviceHandle handle);
            Result GetAdminInfo(DeviceHandle handle, OutPointerWithServerSize<AdminInfo, 1> out_info);
            Result GetRegisterInfo2(DeviceHandle handle, OutPointerWithServerSize<RegisterInfo, 1> out_info);
            Result SetRegisterInfo(DeviceHandle handle, InPointer<RegisterInfo> info);
            Result DeleteRegisterInfo(DeviceHandle handle);
            Result DeleteApplicationArea(DeviceHandle handle);
            Result ExistsApplicationArea(DeviceHandle handle, Out<u8> exists);

        public:
        
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(ISystem, Initialize),
                MAKE_SERVICE_COMMAND_META(ISystem, Finalize),
                MAKE_SERVICE_COMMAND_META(ISystem, ListDevices),
                MAKE_SERVICE_COMMAND_META(ISystem, StartDetection),
                MAKE_SERVICE_COMMAND_META(ISystem, StopDetection),
                MAKE_SERVICE_COMMAND_META(ISystem, Mount),
                MAKE_SERVICE_COMMAND_META(ISystem, Unmount),
                MAKE_SERVICE_COMMAND_META(ISystem, OpenApplicationArea),
                MAKE_SERVICE_COMMAND_META(ISystem, GetApplicationArea),
                MAKE_SERVICE_COMMAND_META(ISystem, SetApplicationArea),
                MAKE_SERVICE_COMMAND_META(ISystem, Flush),
                MAKE_SERVICE_COMMAND_META(ISystem, Restore),
                MAKE_SERVICE_COMMAND_META(ISystem, CreateApplicationArea),
                MAKE_SERVICE_COMMAND_META(ISystem, GetTagInfo),
                MAKE_SERVICE_COMMAND_META(ISystem, GetRegisterInfo),
                MAKE_SERVICE_COMMAND_META(ISystem, GetCommonInfo),
                MAKE_SERVICE_COMMAND_META(ISystem, GetModelInfo),
                MAKE_SERVICE_COMMAND_META(ISystem, AttachActivateEvent),
                MAKE_SERVICE_COMMAND_META(ISystem, AttachDeactivateEvent),
                MAKE_SERVICE_COMMAND_META(ISystem, GetState),
                MAKE_SERVICE_COMMAND_META(ISystem, GetDeviceState),
                MAKE_SERVICE_COMMAND_META(ISystem, GetNpadId),
                MAKE_SERVICE_COMMAND_META(ISystem, AttachAvailabilityChangeEvent),
                MAKE_SERVICE_COMMAND_META(ISystem, GetApplicationAreaSize),
                MAKE_SERVICE_COMMAND_META(ISystem, RecreateApplicationArea),

                MAKE_SERVICE_COMMAND_META(ISystem, Format),
                MAKE_SERVICE_COMMAND_META(ISystem, GetAdminInfo),
                MAKE_SERVICE_COMMAND_META(ISystem, GetRegisterInfo2),
                MAKE_SERVICE_COMMAND_META(ISystem, SetRegisterInfo),
                MAKE_SERVICE_COMMAND_META(ISystem, DeleteRegisterInfo),
                MAKE_SERVICE_COMMAND_META(ISystem, DeleteApplicationArea),
                MAKE_SERVICE_COMMAND_META(ISystem, ExistsApplicationArea),
            };
    };

    void CreateCommonActivateEvent();
}