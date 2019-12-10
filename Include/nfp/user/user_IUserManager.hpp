
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <nfp/user/user_IUser.hpp>

namespace nfp::user
{
    class IUserManager final : public ams::sf::IMitmServiceObject
    {
        public:

            IUserManager(std::shared_ptr<::Service> &&s, const ams::sm::MitmProcessInfo &c);
            static bool ShouldMitm(const ams::sm::MitmProcessInfo &client_info);

        private:

            enum class CommandId
            {
                CreateUserInterface = 0,
            };

            ams::Result CreateUserInterface(ams::sf::Out<std::shared_ptr<IUser>> out);

        public:
        
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(CreateUserInterface),
            };
    };
}