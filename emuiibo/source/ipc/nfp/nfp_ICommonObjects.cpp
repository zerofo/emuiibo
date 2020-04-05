#include <ipc/nfp/nfp_ICommonObjects.hpp>

namespace ipc::nfp {

    static void VirtualAmiiboScanThread(void *iface_data) {
        ICommonInterface *iface_ptr = reinterpret_cast<ICommonInterface*>(iface_data);
        while(true) {
            if(iface_ptr->ShouldExitThread()) {
                break;
            }
            auto status = sys::GetActiveVirtualAmiiboStatus();
            iface_ptr->HandleVirtualAmiiboStatus(status);
            svcSleepThread(100'000'000ul);
        }
    }

    ICommonInterface::ICommonInterface(Service *fwd) : state(NfpState_NonInitialized), device_state(NfpDeviceState_Unavailable), forward_service(fwd), should_exit_thread(false) {
        this->event_activate.InitializeAsInterProcessEvent();
        this->event_deactivate.InitializeAsInterProcessEvent();
        this->event_availability_change.InitializeAsInterProcessEvent();
        EMU_R_ASSERT(this->scan_thread.Initialize(&VirtualAmiiboScanThread, reinterpret_cast<void*>(this), 0x2000, 0x2b));
        EMU_R_ASSERT(this->scan_thread.Start());
    }

    ICommonInterface::~ICommonInterface() {
        serviceClose(this->forward_service);
        delete this->forward_service;
        this->NotifyThreadExitAndWait();
    }

    void ICommonInterface::HandleVirtualAmiiboStatus(sys::VirtualAmiiboStatus status) {
        EMU_LOCK_SCOPE_WITH(this->emu_scan_lock);
        this->last_notified_status = status;
        EMU_LOG_FMT("Got status: " << static_cast<u32>(status))
        // In this context, Invalid status = the status was consumed, waiting for another change
        switch(this->last_notified_status) {
            case sys::VirtualAmiiboStatus::Connected: {
                switch(this->device_state) {
                    case NfpDeviceState_SearchingForTag: {
                        // The client was waiting for an amiibo, tell it that it's connected now
                        EMU_LOG_FMT("The client was waiting for an amiibo, tell it that it's connected now")
                        this->device_state = NfpDeviceState_TagFound;
                        this->event_activate.Signal();
                        this->last_notified_status = sys::VirtualAmiiboStatus::Invalid;
                        break;
                    }
                    case NfpDeviceState_TagFound: {
                        // We already know it's connected
                        EMU_LOG_FMT("We already know it's connected")
                        this->last_notified_status = sys::VirtualAmiiboStatus::Invalid;
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case sys::VirtualAmiiboStatus::Disconnected: {
                switch(this->device_state) {
                    case NfpDeviceState_TagFound:
                    case NfpDeviceState_TagMounted: {
                        // The client thinks that the amiibo is connected, tell it that it was disconnected
                        EMU_LOG_FMT("The client thinks that the amiibo is connected, tell it that it was disconnected")
                        this->device_state = NfpDeviceState_SearchingForTag;
                        this->event_deactivate.Signal();
                        this->last_notified_status = sys::VirtualAmiiboStatus::Invalid;
                        break;
                    }
                    case NfpDeviceState_SearchingForTag: {
                        // We already know it's not connected
                        EMU_LOG_FMT("We already know it's not connected")
                        this->last_notified_status = sys::VirtualAmiiboStatus::Invalid;
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }

    ams::Result ICommonInterface::Initialize(const ams::sf::ClientAppletResourceUserId &client_aruid, const ams::sf::ClientProcessId &client_pid, const ams::sf::InBuffer &mcu_data) {
        EMU_LOG_FMT("Process ID: 0x" << std::hex << client_pid.GetValue().value << ", ARUID: 0x" << std:: hex << client_aruid.GetValue().value)

        this->state = NfpState_Initialized;
        this->device_state = NfpDeviceState_Initialized;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::Finalize() {
        EMU_LOG_FMT("Finalizing...")
        this->state = NfpState_NonInitialized;
        this->device_state = NfpDeviceState_Finalized;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::ListDevices(const ams::sf::OutPointerArray<DeviceHandle> &out_devices, ams::sf::Out<s32> out_count) {
        EMU_LOG_FMT("Device array length: " << out_devices.GetSize())
        u64 id = 0x20;
        hidScanInput();
        if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) {
            id = (u64)CONTROLLER_PLAYER_1;
        }
        DeviceHandle handle = {};
        handle.npad_id = (u32)id;
        out_devices[0] = handle;
        out_count.SetValue(1);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::StartDetection(DeviceHandle handle) {
        EMU_LOG_FMT("Started detection")
        this->device_state = NfpDeviceState_SearchingForTag;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::StopDetection(DeviceHandle handle) {
        EMU_LOG_FMT("Stopped detection")
        /*
        switch(this->device_state) {
            case NfpDeviceState_TagFound:
            case NfpDeviceState_TagMounted:
                this->eventDeactivate.Signal();
            case NfpDeviceState_SearchingForTag:
            case NfpDeviceState_TagRemoved:
                this->device_state = NfpDeviceState_Initialized;
                break;
            default:
                break;
        }
        */
        this->device_state = NfpDeviceState_Initialized;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::Mount(DeviceHandle handle, u32 type, u32 target) {
        EMU_LOG_FMT("Mounted")
        // this->event_activate.Signal();
        this->device_state = NfpDeviceState_TagMounted;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::Unmount(DeviceHandle handle) {
        EMU_LOG_FMT("Unmounted")
        // this->event_deactivate.Signal();
        // this->device_state = NfpDeviceState_SearchingForTag;
        this->device_state = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::Flush(DeviceHandle handle) {
        EMU_LOG_FMT("Flushed")
        this->device_state = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::Restore(DeviceHandle handle) {
        EMU_LOG_FMT("Restored")
        this->device_state = NfpDeviceState_TagFound;
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetTagInfo(ams::sf::Out<TagInfo> out_info, DeviceHandle handle) {
        auto &amiibo = sys::GetActiveVirtualAmiibo();
        EMU_LOG_FMT("Tag info - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", amiibo name: " << amiibo.GetName())
        R_UNLESS(amiibo.IsValid(), result::nfp::ResultAreaNeedsToBeCreated);
        auto info = amiibo.ProduceTagInfo();
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetRegisterInfo(ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle) {
        auto &amiibo = sys::GetActiveVirtualAmiibo();
        EMU_LOG_FMT("Register info - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", amiibo name: " << amiibo.GetName())
        R_UNLESS(amiibo.IsValid(), result::nfp::ResultAreaNeedsToBeCreated);
        auto info = amiibo.ProduceRegisterInfo();
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetModelInfo(ams::sf::Out<ModelInfo> out_info, DeviceHandle handle) {
        auto &amiibo = sys::GetActiveVirtualAmiibo();
        EMU_LOG_FMT("Model info - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", amiibo name: " << amiibo.GetName())
        R_UNLESS(amiibo.IsValid(), result::nfp::ResultAreaNeedsToBeCreated);
        auto info = amiibo.ProduceModelInfo();
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetCommonInfo(ams::sf::Out<CommonInfo> out_info, DeviceHandle handle) {
        auto &amiibo = sys::GetActiveVirtualAmiibo();
        EMU_LOG_FMT("Common info - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", amiibo name: " << amiibo.GetName())
        R_UNLESS(amiibo.IsValid(), result::nfp::ResultAreaNeedsToBeCreated);
        auto info = amiibo.ProduceCommonInfo();
        out_info.SetValue(info);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::AttachActivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event) {
        event.SetValue(event_activate.GetReadableHandle());
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::AttachDeactivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event) {
        event.SetValue(event_deactivate.GetReadableHandle());
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetState(ams::sf::Out<u32> out_state) {
        EMU_LOG_FMT("State: " << static_cast<u32>(this->state));
        out_state.SetValue(static_cast<u32>(this->state));
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetDeviceState(DeviceHandle handle, ams::sf::Out<u32> out_state) {
        EMU_LOG_FMT("Device state: " << static_cast<u32>(this->device_state));
        out_state.SetValue(static_cast<u32>(this->device_state));
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::GetNpadId(DeviceHandle handle, ams::sf::Out<u32> out_npad_id) {
        out_npad_id.SetValue(handle.npad_id);
        return ams::ResultSuccess();
    }

    ams::Result ICommonInterface::AttachAvailabilityChangeEvent(ams::sf::Out<ams::sf::CopyHandle> event) {
        event.SetValue(event_availability_change.GetReadableHandle());
        return ams::ResultSuccess();
    }

    static inline Result _fwd_CreateInterface(Service *out, Service *manager_srv) {
        return serviceDispatch(manager_srv, 0,
            .out_num_objects = 1,
            .out_objects = out,
        );
    }

    ams::Result ICommonManager::CreateForwardInterface(Service *manager, Service *out) {
        R_UNLESS(sys::GetEmulationStatus() == sys::EmulationStatus::On, ams::sm::mitm::ResultShouldForwardToSession());
        R_TRY(_fwd_CreateInterface(out, manager));
        EMU_LOG_FMT("Created MitM'd interface for emuiibo!")
        return ams::ResultSuccess();
    }

}