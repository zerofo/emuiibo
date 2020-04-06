 
#pragma once
#include <sys/sys_Emulation.hpp>
#include <ipc/ipc_Utils.hpp>
#include <emu_Results.hpp>

namespace ipc::emu {

    class IVirtualAmiibo final : public ams::sf::IServiceObject {

        private:
            amiibo::VirtualAmiibo virtual_amiibo;

        private:
            enum class CommandId {
                SetAsActiveAmiibo = 0,
                GetName = 1,
                GetPath = 2,
            };

            void SetAsActiveAmiibo() {
                sys::SetActiveVirtualAmiibo(this->virtual_amiibo);
            }

            void GetName(const ams::sf::OutBuffer &out_name) {
                CopyStringToOutBuffer(this->virtual_amiibo.GetName(), out_name);
            }

            void GetPath(const ams::sf::OutBuffer &out_path) {
                CopyStringToOutBuffer(this->virtual_amiibo.GetPath(), out_path);
            }

        public:
            IVirtualAmiibo(amiibo::VirtualAmiibo amiibo) : virtual_amiibo(amiibo) {}

        public:
            DEFINE_SERVICE_DISPATCH_TABLE {
                MAKE_SERVICE_COMMAND_META(SetAsActiveAmiibo),
                MAKE_SERVICE_COMMAND_META(GetName),
                MAKE_SERVICE_COMMAND_META(GetPath),
            };
    };

}