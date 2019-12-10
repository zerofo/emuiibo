 
#pragma once
#include <nfp/nfp_Types.hpp>

namespace nfp::user
{
    class IUser final : public ams::sf::IServiceObject
    {
        private:

            NfpState state;
            NfpDeviceState deviceState;
            ams::os::SystemEvent eventActivate;
            ams::os::SystemEvent eventDeactivate;
            ams::os::SystemEvent eventAvailabilityChange;
            u32 currentAreaAppId;
            Service fwd_srv;

        public:

            IUser(Service forward_intf);
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

            ams::Result Initialize(u64 aruid, u64 zero, const ams::sf::ClientProcessId &client_pid, const ams::sf::InBuffer &input_ver_data);
            ams::Result Finalize();
            ams::Result ListDevices(const ams::sf::OutPointerArray<DeviceHandle> &out_devices, ams::sf::Out<s32> out_count);
            ams::Result StartDetection(DeviceHandle handle);
            ams::Result StopDetection(DeviceHandle handle);
            ams::Result Mount(DeviceHandle handle, u32 type, u32 target);
            ams::Result Unmount(DeviceHandle handle);
            ams::Result OpenApplicationArea(ams::sf::Out<u32> npad_id, DeviceHandle handle, u32 id);
            ams::Result GetApplicationArea(ams::sf::OutBuffer &data, ams::sf::Out<u32> data_size, DeviceHandle handle);
            ams::Result SetApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle);
            ams::Result Flush(DeviceHandle handle);
            ams::Result Restore(DeviceHandle handle);
            ams::Result CreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, u32 id);
            ams::Result GetTagInfo(DeviceHandle handle, ams::sf::Out<TagInfo> out_info);
            ams::Result GetRegisterInfo(DeviceHandle handle, ams::sf::Out<RegisterInfo> out_info);
            ams::Result GetCommonInfo(DeviceHandle handle, ams::sf::Out<CommonInfo> out_info);
            ams::Result GetModelInfo(DeviceHandle handle, ams::sf::Out<ModelInfo> out_info);
            ams::Result AttachActivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event);
            ams::Result AttachDeactivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event);
            ams::Result GetState(ams::sf::Out<u32> state);
            ams::Result GetDeviceState(DeviceHandle handle, ams::sf::Out<u32> state);
            ams::Result GetNpadId(DeviceHandle handle, ams::sf::Out<u32> npad_id);
            ams::Result GetApplicationAreaSize(DeviceHandle handle, ams::sf::Out<u32> size);
            ams::Result AttachAvailabilityChangeEvent(ams::sf::Out<ams::sf::CopyHandle> event);
            ams::Result RecreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, u32 id);

        public:
        
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(Initialize),
                MAKE_SERVICE_COMMAND_META(Finalize),
                MAKE_SERVICE_COMMAND_META(ListDevices),
                MAKE_SERVICE_COMMAND_META(StartDetection),
                MAKE_SERVICE_COMMAND_META(StopDetection),
                MAKE_SERVICE_COMMAND_META(Mount),
                MAKE_SERVICE_COMMAND_META(Unmount),
                MAKE_SERVICE_COMMAND_META(OpenApplicationArea),
                MAKE_SERVICE_COMMAND_META(GetApplicationArea),
                MAKE_SERVICE_COMMAND_META(SetApplicationArea),
                MAKE_SERVICE_COMMAND_META(Flush),
                MAKE_SERVICE_COMMAND_META(Restore),
                MAKE_SERVICE_COMMAND_META(CreateApplicationArea),
                MAKE_SERVICE_COMMAND_META(GetTagInfo),
                MAKE_SERVICE_COMMAND_META(GetRegisterInfo),
                MAKE_SERVICE_COMMAND_META(GetCommonInfo),
                MAKE_SERVICE_COMMAND_META(GetModelInfo),
                MAKE_SERVICE_COMMAND_META(AttachActivateEvent),
                MAKE_SERVICE_COMMAND_META(AttachDeactivateEvent),
                MAKE_SERVICE_COMMAND_META(GetState),
                MAKE_SERVICE_COMMAND_META(GetDeviceState),
                MAKE_SERVICE_COMMAND_META(GetNpadId),
                MAKE_SERVICE_COMMAND_META(AttachAvailabilityChangeEvent),
                MAKE_SERVICE_COMMAND_META(GetApplicationAreaSize),
                MAKE_SERVICE_COMMAND_META(RecreateApplicationArea),
            };
    };

    void CreateCommonActivateEvent();
}