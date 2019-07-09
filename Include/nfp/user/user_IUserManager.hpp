
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <nfp/user/user_IUser.hpp>

namespace nfp::user
{
    class IUserManager final : public IMitmServiceObject
    {
        public:

            IUserManager(std::shared_ptr<Service> s, u64 pid, sts::ncm::TitleId tid);
            static bool ShouldMitm(u64 pid, sts::ncm::TitleId tid);
            static void PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx);  

        private:

            enum class CommandId
            {
                CreateUserInterface = 0,
            };

            Result CreateUserInterface(Out<std::shared_ptr<IUser>> out);

        public:
        
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(IUserManager, CreateUserInterface),
            };
    };
}