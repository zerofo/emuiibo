 
#pragma once
#include <sys/sys_Emulation.hpp>
#include <emu_Results.hpp>

namespace ipc::emu {

    class IVirtualAmiibo final : public ams::sf::IServiceObject {

        private:
            amiibo::VirtualAmiibo virtual_amiibo;

        private:
            enum class CommandId {
                SetAsActiveAmiibo = 0,
                GetName = 1,
            };

            void SetAsActiveAmiibo() {
                sys::SetActiveVirtualAmiibo(this->virtual_amiibo);
            }

            void GetName(const ams::sf::OutBuffer &out_name) {
                EMU_LOG_FMT("Out name buffer size: " << out_name.GetSize())
                auto name = this->virtual_amiibo.GetName();
                EMU_LOG_FMT("Name: " << name)
                const size_t buf_len = std::min(name.length(), out_name.GetSize());
                strncpy(reinterpret_cast<char*>(out_name.GetPointer()), name.c_str(), buf_len);
            }

        public:
            IVirtualAmiibo(amiibo::VirtualAmiibo amiibo) : virtual_amiibo(amiibo) {}

        public:
            DEFINE_SERVICE_DISPATCH_TABLE {
                MAKE_SERVICE_COMMAND_META(SetAsActiveAmiibo),
                MAKE_SERVICE_COMMAND_META(GetName),
            };
    };

}