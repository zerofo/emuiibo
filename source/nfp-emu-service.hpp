
#pragma once
#include <switch.h>
#include <stratosphere.hpp>

enum NfpEmulationCmd : u32
{
    NfpEmulationCmd_GetAmiiboCount = 0,
    NfpEmulationCmd_GetCurrentAmiibo = 1,
    NfpEmulationCmd_RequestUseCustomAmiibo = 2,
    NfpEmulationCmd_RequestResetCustomAmiibo = 3,
    NfpEmulationCmd_Toggle = 4,
    NfpEmulationCmd_ToggleOnce = 5,
    NfpEmulationCmd_Untoggle = 6,
    NfpEmulationCmd_SwapNext = 7,
};

class NfpEmulationService : public IServiceObject
{
    protected:
        Result GetAmiiboCount(Out<u32> out);
        Result GetCurrentAmiibo(Out<u32> idx);
        Result RequestUseCustomAmiibo(InBuffer<char> Path);
        Result RequestResetCustomAmiibo();
        Result Toggle();
        Result ToggleOnce();
        Result Untoggle();
        Result SwapNext();
        /*
        Result GetCurrentAmiibo(OutBuffer<char> path, Out<bool> ok);
        Result SetCustomAmiibo(InBuffer<char> path);
        Result ResetCustomAmiibo();
        Result Toggle();
        Result ToggleOnce();
        Result Untoggle();
        Result SwapNext();
        Result GetToggleStatus(Out<u32> status);
        */
    public:
        DEFINE_SERVICE_DISPATCH_TABLE
        {
            MakeServiceCommandMeta<NfpEmulationCmd_GetAmiiboCount, &NfpEmulationService::GetAmiiboCount>(),
            MakeServiceCommandMeta<NfpEmulationCmd_GetCurrentAmiibo, &NfpEmulationService::GetCurrentAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_RequestUseCustomAmiibo, &NfpEmulationService::RequestUseCustomAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_RequestResetCustomAmiibo, &NfpEmulationService::RequestResetCustomAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_Toggle, &NfpEmulationService::Toggle>(),
            MakeServiceCommandMeta<NfpEmulationCmd_ToggleOnce, &NfpEmulationService::ToggleOnce>(),
            MakeServiceCommandMeta<NfpEmulationCmd_Untoggle, &NfpEmulationService::Untoggle>(),
            MakeServiceCommandMeta<NfpEmulationCmd_SwapNext, &NfpEmulationService::SwapNext>(),
        };
};