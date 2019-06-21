
#pragma once
#include <switch.h>
#include <stratosphere.hpp>

enum NfpEmulationCmd : u32
{
    NfpEmulationCmd_GetAmiibo = 0,
    NfpEmulationCmd_SetAmiibo = 1,
    NfpEmulationCmd_ResetAmiibo = 2,
    NfpEmulationCmd_Toggle = 3,
    NfpEmulationCmd_ToggleOnce = 4,
    NfpEmulationCmd_Untoggle = 5,
    NfpEmulationCmd_SwapNext = 6,
    NfpEmulationCmd_GetToggleStatus = 7,
    NfpEmulationCmd_RescanAmiibos = 8,
};

class NfpEmulationService : public IServiceObject
{
    protected:
        Result GetAmiibo(OutBuffer<char> path, Out<bool> ok);
        Result SetAmiibo(InBuffer<char> path);
        Result ResetAmiibo();
        Result Toggle();
        Result ToggleOnce();
        Result Untoggle();
        Result SwapNext();
        Result GetToggleStatus(Out<u32> status);
        Result RescanAmiibos();
    public:
        DEFINE_SERVICE_DISPATCH_TABLE
        {
            MakeServiceCommandMeta<NfpEmulationCmd_GetAmiibo, &NfpEmulationService::GetAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_SetAmiibo, &NfpEmulationService::SetAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_ResetAmiibo, &NfpEmulationService::ResetAmiibo>(),
            MakeServiceCommandMeta<NfpEmulationCmd_Toggle, &NfpEmulationService::Toggle>(),
            MakeServiceCommandMeta<NfpEmulationCmd_ToggleOnce, &NfpEmulationService::ToggleOnce>(),
            MakeServiceCommandMeta<NfpEmulationCmd_Untoggle, &NfpEmulationService::Untoggle>(),
            MakeServiceCommandMeta<NfpEmulationCmd_SwapNext, &NfpEmulationService::SwapNext>(),
            MakeServiceCommandMeta<NfpEmulationCmd_GetToggleStatus, &NfpEmulationService::GetToggleStatus>(),
            MakeServiceCommandMeta<NfpEmulationCmd_RescanAmiibos, &NfpEmulationService::RescanAmiibos>(),
        };
};