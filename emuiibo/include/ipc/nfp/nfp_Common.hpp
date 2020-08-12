 
#pragma once
#include <ipc/nfp/nfp_Types.hpp>
#include <emu_Results.hpp>
#include <sys/sys_Emulation.hpp>

namespace ipc::nfp {

    namespace impl {

        using namespace ams;

        #define I_COMMON_INTERFACE_INTERFACE_INFO(C, H)                                                        \
            AMS_SF_METHOD_INFO(C, H,  0, void, Initialize,             (const ams::sf::ClientAppletResourceUserId &client_aruid, const ams::sf::ClientProcessId &client_pid, const ams::sf::InBuffer &mcu_data))                                             \
            AMS_SF_METHOD_INFO(C, H,  1, void, Finalize,             ())                                             \
            AMS_SF_METHOD_INFO(C, H,  2, ams::Result, ListDevices,             (const ams::sf::OutPointerArray<DeviceHandle> &out_devices, ams::sf::Out<s32> out_count))                                             \
            AMS_SF_METHOD_INFO(C, H,  3, ams::Result, StartDetection,             (DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  4, ams::Result, StopDetection,             (DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  5, ams::Result, Mount,             (DeviceHandle handle, u32 type, u32 target))                                             \
            AMS_SF_METHOD_INFO(C, H,  6, ams::Result, Unmount,             (DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  10, ams::Result, Flush,             (DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  11, ams::Result, Restore,             (DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  13, ams::Result, GetTagInfo,             (ams::sf::Out<TagInfo> out_info, DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  14, ams::Result, GetRegisterInfo,             (ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  15, ams::Result, GetCommonInfo,             (ams::sf::Out<CommonInfo> out_info, DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  16, ams::Result, GetModelInfo,             (ams::sf::Out<ModelInfo> out_info, DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  17, ams::Result, AttachActivateEvent,             (DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event))                                             \
            AMS_SF_METHOD_INFO(C, H,  18, ams::Result, AttachDeactivateEvent,             (DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event))                                             \
            AMS_SF_METHOD_INFO(C, H,  19, void, GetState,             (ams::sf::Out<u32> state))                                             \
            AMS_SF_METHOD_INFO(C, H,  20, void, GetDeviceState,             (DeviceHandle handle, ams::sf::Out<u32> state))                                             \
            AMS_SF_METHOD_INFO(C, H,  21, ams::Result, GetNpadId,             (DeviceHandle handle, ams::sf::Out<u32> npad_id))                                             \
            AMS_SF_METHOD_INFO(C, H,  23, ams::Result, AttachAvailabilityChangeEvent,             (ams::sf::Out<ams::sf::CopyHandle> event))

        AMS_SF_DEFINE_INTERFACE(ICommonInterface, I_COMMON_INTERFACE_INTERFACE_INFO)

        #define I_MANAGER_INTERFACE_INFO(C, H)                                                        \
            AMS_SF_METHOD_INFO(C, H,  0, ams::Result, CreateInterface,             (ams::sf::Out<std::shared_ptr<ICommonInterface>> out))

        AMS_SF_DEFINE_MITM_INTERFACE(IManager, I_MANAGER_INTERFACE_INFO)
    }

    class CommonInterface {

        public:
            static constexpr u32 HandheldNpadId = 0x20;
            static constexpr u32 Player1NpadId = 0;

        protected:
            NfpState state;
            NfpDeviceState device_state;
            ams::os::SystemEventType event_activate;
            ams::os::SystemEventType event_deactivate;
            ams::os::SystemEventType event_availability_change;
            Service forward_service;
            u64 client_app_id;

            Lock amiibo_update_lock;
            Thread amiibo_update_thread;
            bool should_exit_thread;

        protected:
            NfpState GetStateValue();
            void SetStateValue(NfpState val);
            NfpDeviceState GetDeviceStateValue();
            void SetDeviceStateValue(NfpDeviceState val);

            inline void NotifyShouldExitThread() {
                EMU_LOCK_SCOPE_WITH(this->amiibo_update_lock);
                this->should_exit_thread = true;
            }

            inline void NotifyThreadExitAndWait() {
                this->NotifyShouldExitThread();
                EMU_R_ASSERT(threadWaitForExit(&this->amiibo_update_thread));
            }

            template<typename T>
            inline constexpr void IsInStateImpl(bool &out, T base, T state) {
                if(base == state) {
                    out = true;
                }
            }

            template<typename T, typename ...Ss>
            inline constexpr bool IsStateAny(Ss &&...states) {
                static_assert(std::is_same_v<T, NfpState> || std::is_same_v<T, NfpDeviceState>, "Invalid type");
                bool ret = false;
                if constexpr(std::is_same_v<T, NfpState>) {
                    auto state = this->GetStateValue();
                    (IsInStateImpl(ret, state, states), ...);
                    return ret;
                }
                else if constexpr(std::is_same_v<T, NfpDeviceState>) {
                    auto state = this->GetDeviceStateValue();
                    (IsInStateImpl(ret, state, states), ...);
                    return ret;
                }
                return false;
            }

        public:
            CommonInterface(Service fwd, u64 app_id);
            virtual ~CommonInterface();

            void HandleVirtualAmiiboStatus(sys::VirtualAmiiboStatus status);

            inline bool ShouldExitThread() {
                EMU_LOCK_SCOPE_WITH(this->amiibo_update_lock);
                return this->should_exit_thread;
            }

        public:
            void Initialize(const ams::sf::ClientAppletResourceUserId &client_aruid, const ams::sf::ClientProcessId &client_pid, const ams::sf::InBuffer &mcu_data);
            void Finalize();
            ams::Result ListDevices(const ams::sf::OutPointerArray<DeviceHandle> &out_devices, ams::sf::Out<s32> out_count);
            ams::Result StartDetection(DeviceHandle handle);
            ams::Result StopDetection(DeviceHandle handle);
            ams::Result Mount(DeviceHandle handle, u32 type, u32 target);
            ams::Result Unmount(DeviceHandle handle);
            ams::Result Flush(DeviceHandle handle);
            ams::Result Restore(DeviceHandle handle);
            ams::Result GetTagInfo(ams::sf::Out<TagInfo> out_info, DeviceHandle handle);
            ams::Result GetRegisterInfo(ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle);
            ams::Result GetCommonInfo(ams::sf::Out<CommonInfo> out_info, DeviceHandle handle);
            ams::Result GetModelInfo(ams::sf::Out<ModelInfo> out_info, DeviceHandle handle);
            ams::Result AttachActivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event);
            ams::Result AttachDeactivateEvent(DeviceHandle handle, ams::sf::Out<ams::sf::CopyHandle> event);
            void GetState(ams::sf::Out<u32> state);
            void GetDeviceState(DeviceHandle handle, ams::sf::Out<u32> state);
            ams::Result GetNpadId(DeviceHandle handle, ams::sf::Out<u32> npad_id);
            ams::Result AttachAvailabilityChangeEvent(ams::sf::Out<ams::sf::CopyHandle> event);

    };
    static_assert(impl::IsICommonInterface<CommonInterface>);

    class ManagerBase : public ams::sf::MitmServiceImplBase {

        protected:
            static ams::Result CreateForwardInterface(Service *manager, Service *out);

            template<typename I, typename T>
            inline ams::Result CreateInterfaceImpl(ams::sf::Out<std::shared_ptr<I>> &out) {
                EMU_LOG_FMT("Application ID 0x" << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << this->client_info.program_id.value);
                
                Service outsrv;
                R_TRY(CreateForwardInterface(this->forward_service.get(), &outsrv));

                const ams::sf::cmif::DomainObjectId object_id { serviceGetObjectId(&outsrv) };
                EMU_LOG_FMT("MakeShared done");
                out.SetValue(ams::sf::MakeShared<I, T>(outsrv, this->client_info.program_id.value), object_id);
                EMU_LOG_FMT("Returning success...");
                return ams::ResultSuccess();
            }

        public:
            using MitmServiceImplBase::MitmServiceImplBase;

            static bool ShouldMitm(const ams::sm::MitmProcessInfo &client_info) {
                return sys::GetEmulationStatus() == sys::EmulationStatus::On;
            }

    };

}