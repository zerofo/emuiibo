 
#pragma once
#include <ipc/emu/emu_IVirtualAmiibo.hpp>
#include <sys/sys_Locator.hpp>

namespace ipc::emu {

    constexpr ams::sm::ServiceName ServiceName = ams::sm::ServiceName::Encode("nfp:emu");

    class IEmulationService final : public ams::sf::IServiceObject {

        private:
            enum class CommandId {
                GetEmulationStatus = 0,
                SetEmulationStatus = 1,
                GetActiveVirtualAmiibo = 2,
                ResetActiveVirtualAmiibo = 3,
                GetActiveVirtualAmiiboStatus = 4,
                SetActiveVirtualAmiiboStatus = 5,
                GetVirtualAmiiboCount = 6,
                OpenVirtualAmiibo = 7,
                GetVersion = 8,
            };

            inline ams::Result OpenAmiiboImpl(amiibo::VirtualAmiibo amiibo, ams::sf::Out<std::shared_ptr<IVirtualAmiibo>> out_amiibo) {
                EMU_LOG_FMT("Virtual amiibo valid: " << std::boolalpha << amiibo.IsValid())
                R_UNLESS(amiibo.IsValid(), 0xdead);
                EMU_LOG_FMT("Amiibo name: '" << amiibo.GetName() << "'")

                auto amiibo_obj = std::make_shared<IVirtualAmiibo>(amiibo);
                out_amiibo.SetValue(std::move(amiibo_obj));
                return ams::ResultSuccess();
            }

        private:
            void GetEmulationStatus(ams::sf::Out<sys::EmulationStatus> out_status) {
                auto status = sys::GetEmulationStatus();
                EMU_LOG_FMT("Emulation status: " << static_cast<u32>(status))
                out_status.SetValue(status);
            }

            void SetEmulationStatus(sys::EmulationStatus status) {
                EMU_LOG_FMT("Emulation status: " << static_cast<u32>(status))
                sys::SetEmulationStatus(status);
            }

            ams::Result GetActiveVirtualAmiibo(ams::sf::Out<std::shared_ptr<IVirtualAmiibo>> out_amiibo) {
                auto &amiibo = sys::GetActiveVirtualAmiibo();
                return OpenAmiiboImpl(amiibo, out_amiibo);
            }

            void ResetActiveVirtualAmiibo() {
                EMU_LOG_FMT("Resetting active virtual amiibo...")
                amiibo::VirtualAmiibo empty_amiibo;
                sys::SetActiveVirtualAmiibo(empty_amiibo);
            }

            void GetActiveVirtualAmiiboStatus(ams::sf::Out<sys::VirtualAmiiboStatus> out_status) {
                auto status = sys::GetActiveVirtualAmiiboStatus();
                EMU_LOG_FMT("Virtual amiibo status: " << static_cast<u32>(status))
                out_status.SetValue(status);
            }

            void SetActiveVirtualAmiiboStatus(sys::VirtualAmiiboStatus status) {
                EMU_LOG_FMT("Virtual amiibo status: " << static_cast<u32>(status))
                sys::SetActiveVirtualAmiiboStatus(status);
            }

            void GetVirtualAmiiboCount(ams::sf::Out<u32> out_count) {
                auto count = sys::GetVirtualAmiiboCount();
                EMU_LOG_FMT("Count: " << count)
                out_count.SetValue(count);
            }

            ams::Result OpenVirtualAmiibo(u32 idx, ams::sf::Out<std::shared_ptr<IVirtualAmiibo>> out_amiibo) {
                auto amiibo_path = sys::GetVirtualAmiibo(idx);
                amiibo::VirtualAmiibo amiibo(amiibo_path);
                return OpenAmiiboImpl(amiibo, out_amiibo);
            }

            void GetVersion(ams::sf::Out<Version> out_version) {
                out_version.SetValue(CurrentVersion);
            }
        
        public:
            IEmulationService() {
                sys::UpdateVirtualAmiiboCache();
            }

        public:
            DEFINE_SERVICE_DISPATCH_TABLE {
                MAKE_SERVICE_COMMAND_META(GetEmulationStatus),
                MAKE_SERVICE_COMMAND_META(SetEmulationStatus),
                MAKE_SERVICE_COMMAND_META(GetActiveVirtualAmiibo),
                MAKE_SERVICE_COMMAND_META(ResetActiveVirtualAmiibo),
                MAKE_SERVICE_COMMAND_META(GetActiveVirtualAmiiboStatus),
                MAKE_SERVICE_COMMAND_META(SetActiveVirtualAmiiboStatus),
                MAKE_SERVICE_COMMAND_META(GetVirtualAmiiboCount),
                MAKE_SERVICE_COMMAND_META(OpenVirtualAmiibo),
                MAKE_SERVICE_COMMAND_META(GetVersion),
            };
    };

}