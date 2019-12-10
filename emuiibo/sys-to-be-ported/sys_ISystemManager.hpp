
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <nfp/sys/sys_ISystem.hpp>

namespace nfp::sys
{
    class ISystemManager final : public IMitmServiceObject
    {
        public:

            ISystemManager(std::shared_ptr<Service> s, u64 pid, sts::ncm::TitleId tid);
            static bool ShouldMitm(u64 pid, sts::ncm::TitleId tid);
            static void PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx);  

        private:

            enum class CommandId
            {
                CreateSystemInterface = 0,
            };

            Result CreateSystemInterface(Out<std::shared_ptr<ISystem>> out);

        public:
        
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(ISystemManager, CreateSystemInterface),
            };
    };
}