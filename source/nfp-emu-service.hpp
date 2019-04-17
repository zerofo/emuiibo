
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
            MakeServiceCommandMeta<NfpEmulationCmd_GetAmiiboCount, &NfpEmulationService::GetAmiiboCount>(),
            MakeServiceCommandMeta<NfpEmulationCmd_GetCurrentAmiibo, &NfpEmulationService::GetCurrentAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_RequestUseCustomAmiibo, &NfpEmulationService::RequestUseCustomAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_RequestResetCustomAmiibo, &NfpEmulationService::RequestResetCustomAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_Toggle, &NfpEmulationService::Toggle>(),
            MakeServiceCommandMeta<NfpEmulationCmd_ToggleOnce, &NfpEmulationService::ToggleOnce>(),
            MakeServiceCommandMeta<NfpEmulationCmd_SwapNext, &NfpEmulationService::SwapNext>(),
        };
};