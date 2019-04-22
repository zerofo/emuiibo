
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include "nfp-iuser.hpp"

enum NfpUserCmd : u32
{
    NfpUserCmd_CreateUserInterface = 0,
};

class NfpUserMitmService : public IMitmServiceObject
{      
    public:
        NfpUserMitmService(std::shared_ptr<Service> s, u64 pid) : IMitmServiceObject(s, pid)
        {
        }
        
        static bool ShouldMitm(u64 pid, u64 tid);
        static void PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx);  
    protected:
        Result CreateUserInterface(Out<std::shared_ptr<NfpIUser>> out);
    public:
        DEFINE_SERVICE_DISPATCH_TABLE
        {
            MakeServiceCommandMeta<NfpUserCmd_CreateUserInterface, &NfpUserMitmService::CreateUserInterface>(),
        };
};
