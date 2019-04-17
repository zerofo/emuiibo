
#pragma once
#include <switch.h>
#include <stratosphere.hpp>

enum NfpEmulationCmd : u32 {
    NfpEmulationCmd_GetAmiiboCount = 0,
    NfpEmulationCmd_GetCurrentAmiibo = 1,
    NfpEmulationCmd_RequestUseCustomAmiibo = 2,
    NfpEmulationCmd_RequestResetCustomAmiibo = 3,
    NfpEmulationCmd_Toggle = 4,
    NfpEmulationCmd_ToggleOnce = 5,
    NfpEmulationCmd_SwapNext = 6,
};

class NfpEmulationService : public IServiceObject {
    protected:
        Result GetAmiiboCount(Out<u32> out);
        Result GetCurrentAmiibo(Out<u32> idx);
        Result RequestUseCustomAmiibo(InBuffer<char> Path);
        Result RequestResetCustomAmiibo();
        Result Toggle();     // Combo action
        Result ToggleOnce(); // Combo action
        Result SwapNext();   // Combo action
    public:
        DEFINE_SERVICE_DISPATCH_TABLE {
            MakeServiceCommandMeta<NfpEmulationCmd_GetAmiiboCount, &NfpHomebrewService::GetAmiiboCount>(),
            MakeServiceCommandMeta<NfpEmulationCmd_GetCurrentAmiibo, &NfpHomebrewService::GetCurrentAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_RequestUseCustomAmiibo, &NfpHomebrewService::RequestUseCustomAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_RequestResetCustomAmiibo, &NfpHomebrewService::RequestResetCustomAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_Toggle, &NfpHomebrewService::Toggle>(),
            MakeServiceCommandMeta<NfpEmulationCmd_ToggleOnce, &NfpHomebrewService::ToggleOnce>(),
            MakeServiceCommandMeta<NfpEmulationCmd_SwapNext, &NfpHomebrewService::SwapNext>(),
        };
};