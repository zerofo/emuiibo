
#pragma once
#include <switch.h>
#include <stratosphere.hpp>

enum NfpHomebrewCmd : u32 {
    NfpHomebrewCmd_GetAmiiboCount,
    NfpHomebrewCmd_GetCurrentAmiibo,
    NfpHomebrewCmd_RequestCustomAmiibo,
    NfpHomebrewCmd_RequestResetCustomAmiibo,
};

class NfpHomebrewService : public IServiceObject {
    protected:
        Result GetAmiiboCount(Out<u32> out);
        Result GetCurrentAmiibo(Out<u32> idx);
        Result RequestCustomAmiibo(InBuffer<char> Path);
        Result RequestResetCustomAmiibo();
    public:
        DEFINE_SERVICE_DISPATCH_TABLE {
            MakeServiceCommandMeta<NfpHomebrewCmd_GetAmiiboCount, &NfpHomebrewService::GetAmiiboCount>(),
            MakeServiceCommandMeta<NfpHomebrewCmd_GetCurrentAmiibo, &NfpHomebrewService::GetCurrentAmiibo>(),
            MakeServiceCommandMeta<NfpHomebrewCmd_RequestCustomAmiibo, &NfpHomebrewService::RequestCustomAmiibo>(),
            MakeServiceCommandMeta<NfpHomebrewCmd_RequestResetCustomAmiibo, &NfpHomebrewService::RequestResetCustomAmiibo>(),
        };
};