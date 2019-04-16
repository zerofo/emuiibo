
#pragma once
#include <switch.h>
#include <stratosphere.hpp>

enum NfpHomebrewCmd : u32 {
    NfpHomebrewCmd_GetAmiiboCount,
    NfpHomebrewCmd_GetCurrentAmiibo,
};

class NfpHomebrewService : public IServiceObject {
    protected:
        Result GetAmiiboCount(Out<u32> out);
        Result GetCurrentAmiibo(Out<u32> idx);
    public:
        DEFINE_SERVICE_DISPATCH_TABLE {
            MakeServiceCommandMeta<NfpHomebrewCmd_GetAmiiboCount, &NfpHomebrewService::GetAmiiboCount>(),
            MakeServiceCommandMeta<NfpHomebrewCmd_GetCurrentAmiibo, &NfpHomebrewService::GetCurrentAmiibo>(),
        };
};