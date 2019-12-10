
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include "nfp/user/user_IUser.hpp"
#include "emu/emu_Status.hpp"
#include "emu/emu_Emulation.hpp"

namespace nfp::user
{
    class IUserManager : public ams::sf::IMitmServiceObject
    {
        private:
            enum class CommandId
            {
                CreateUserInterface = 0,
            };

        public:
            IUserManager(std::shared_ptr<::Service> &&s, const ams::sm::MitmProcessInfo &c) : IMitmServiceObject(std::forward<std::shared_ptr<::Service>>(s), c)
            {
                LOG_FMT("Accessed nfp:user with app ID " << std::hex << std::setw(16) << std::setfill('0') << c.program_id.value)
                emu::SetCurrentAppId(c.program_id.value);
            }

            static bool ShouldMitm(const ams::sm::MitmProcessInfo &client_info)
            {
                LOG_FMT("Should MitM -> status on: " << std::boolalpha << emu::IsStatusOn())
                // We only MitM NFP if emulation is on.
                return emu::IsStatusOn();
            }

        protected:
            ams::Result CreateUserInterface(ams::sf::Out<std::shared_ptr<IUser>> out);

        public:
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(CreateUserInterface),
            };
    };
}