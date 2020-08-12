 
#pragma once
#include <ipc/ipc_Utils.hpp>
#include <sys/sys_Emulation.hpp>

namespace ipc::emu {

    namespace impl {

        using namespace ams;

        #define I_EMULATION_SERVICE_INTERFACE_INFO(C, H)                                                        \
            AMS_SF_METHOD_INFO(C, H,  0, void, GetVersion,             (ams::sf::Out<Version> out_version))                                             \
            AMS_SF_METHOD_INFO(C, H,  1, void, GetVirtualAmiiboDirectory,             (const ams::sf::OutBuffer &out_path_buf))                                             \
            AMS_SF_METHOD_INFO(C, H,  2, void, GetEmulationStatus,             (ams::sf::Out<sys::EmulationStatus> out_status))                                             \
            AMS_SF_METHOD_INFO(C, H,  3, void, SetEmulationStatus,             (sys::EmulationStatus status))                                             \
            AMS_SF_METHOD_INFO(C, H,  4, ams::Result, GetActiveVirtualAmiibo,             (ams::sf::Out<amiibo::VirtualAmiiboData> out_data, const ams::sf::OutBuffer &out_path_buf))                                             \
            AMS_SF_METHOD_INFO(C, H,  5, ams::Result, SetActiveVirtualAmiibo,             (const ams::sf::InBuffer &path_buf))                                             \
            AMS_SF_METHOD_INFO(C, H,  6, void, ResetActiveVirtualAmiibo,             ())                                             \
            AMS_SF_METHOD_INFO(C, H,  7, void, GetActiveVirtualAmiiboStatus,             (ams::sf::Out<sys::VirtualAmiiboStatus> out_status))                                             \
            AMS_SF_METHOD_INFO(C, H,  8, void, SetActiveVirtualAmiiboStatus,             (sys::VirtualAmiiboStatus status))                                             \
            AMS_SF_METHOD_INFO(C, H,  9, void, IsApplicationIdIntercepted,             (ams::sf::Out<bool> out_intercepted, u64 app_id))                                             \
            AMS_SF_METHOD_INFO(C, H,  10, void, IsCurrentApplicationIdIntercepted,             (ams::sf::Out<bool> out_intercepted))                                             \
            AMS_SF_METHOD_INFO(C, H,  11, ams::Result, TryParseVirtualAmiibo,             (const ams::sf::InBuffer &path_buf, ams::sf::Out<amiibo::VirtualAmiiboData> out_data))

        AMS_SF_DEFINE_INTERFACE(IEmulationService, I_EMULATION_SERVICE_INTERFACE_INFO)

    }

    constexpr ams::sm::ServiceName ServiceName = ams::sm::ServiceName::Encode("emuiibo");

    class EmulationService final {

        public:
            void GetVersion(ams::sf::Out<Version> out_version) {
                out_version.SetValue(CurrentVersion);
            }

            void GetVirtualAmiiboDirectory(const ams::sf::OutBuffer &out_path_buf) {
                CopyStringToOutBuffer(consts::AmiiboDir, out_path_buf);
            }

            void GetEmulationStatus(ams::sf::Out<sys::EmulationStatus> out_status) {
                auto status = sys::GetEmulationStatus();
                EMU_LOG_FMT("Emulation status: " << static_cast<u32>(status))
                out_status.SetValue(status);
            }

            void SetEmulationStatus(sys::EmulationStatus status) {
                EMU_LOG_FMT("Emulation status: " << static_cast<u32>(status))
                sys::SetEmulationStatus(status);
            }

            ams::Result GetActiveVirtualAmiibo(ams::sf::Out<amiibo::VirtualAmiiboData> out_data, const ams::sf::OutBuffer &out_path_buf) {
                auto &amiibo = sys::GetActiveVirtualAmiibo();
                R_UNLESS(amiibo.IsValid(), result::emu::ResultNoActiveVirtualAmiibo);

                out_data.SetValue(amiibo.ProduceData());
                CopyStringToOutBuffer(amiibo.GetPath(), out_path_buf);
                return ams::ResultSuccess();
            }

            ams::Result SetActiveVirtualAmiibo(const ams::sf::InBuffer &path_buf) {
                auto path = CopyStringFromInBuffer(path_buf);
                amiibo::VirtualAmiibo amiibo;
                R_UNLESS(amiibo::VirtualAmiibo::GetValidVirtualAmiibo(path, amiibo), result::emu::ResultInvalidVirtualAmiibo);

                sys::SetActiveVirtualAmiibo(amiibo);
                return ams::ResultSuccess();
            }

            void ResetActiveVirtualAmiibo() {
                amiibo::VirtualAmiibo empty_amiibo;
                sys::SetActiveVirtualAmiibo(empty_amiibo);
            }

            void GetActiveVirtualAmiiboStatus(ams::sf::Out<sys::VirtualAmiiboStatus> out_status) {
                auto status = sys::GetActiveVirtualAmiiboStatus();
                out_status.SetValue(status);
            }

            void SetActiveVirtualAmiiboStatus(sys::VirtualAmiiboStatus status) {
                sys::SetActiveVirtualAmiiboStatus(status);
            }

            void IsApplicationIdIntercepted(ams::sf::Out<bool> out_intercepted, u64 app_id) {
                out_intercepted.SetValue(sys::IsApplicationIdIntercepted(app_id));
            }

            void IsCurrentApplicationIdIntercepted(ams::sf::Out<bool> out_intercepted) {
                out_intercepted.SetValue(sys::IsCurrentApplicationIdIntercepted());
            }

            ams::Result TryParseVirtualAmiibo(const ams::sf::InBuffer &path_buf, ams::sf::Out<amiibo::VirtualAmiiboData> out_data) {
                auto path = CopyStringFromInBuffer(path_buf);
                amiibo::VirtualAmiibo amiibo;
                R_UNLESS(amiibo::VirtualAmiibo::GetValidVirtualAmiibo(path, amiibo), result::emu::ResultInvalidVirtualAmiibo);

                out_data.SetValue(amiibo.ProduceData());
                return ams::ResultSuccess();
            }

    };
    static_assert(impl::IsIEmulationService<EmulationService>);

}