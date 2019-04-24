
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include "nfp-isystem.hpp"

enum NfpSystemCmd : u32
{
    NfpSystemCmd_CreateSystemInterface = 0,
};

class NfpSystemMitmService : public IMitmServiceObject
{      
    public:
        NfpSystemMitmService(std::shared_ptr<Service> s, u64 pid) : IMitmServiceObject(s, pid)
        {
        }
        
        static bool ShouldMitm(u64 pid, u64 tid);
        static void PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx);  
    protected:
        Result CreateSystemInterface(Out<std::shared_ptr<NfpISystem>> out);
    public:
        DEFINE_SERVICE_DISPATCH_TABLE
        {
            MakeServiceCommandMeta<NfpSystemCmd_CreateSystemInterface, &NfpSystemMitmService::CreateSystemInterface>(),
        };
};
