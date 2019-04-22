#include "nfp-emu-service.hpp"
#include "emu-amiibo.hpp"

extern u32 g_toggleEmulation;

Result NfpEmulationService::GetAmiiboCount(Out<u32> out)
{
    u32 c = AmiiboEmulator::GetCount();
    out.SetValue(c);
    return 0;
}

Result NfpEmulationService::GetCurrentAmiibo(Out<u32> idx)
{
    s32 cidx = AmiiboEmulator::GetCurrentIndex();
    if(cidx < 0) return LibnxError_NotFound;
    idx.SetValue(cidx);
    return 0;
}

Result NfpEmulationService::RequestUseCustomAmiibo(InBuffer<char> path)
{
    AmiiboEmulator::SetCustomAmiibo(std::string(path.buffer));
    return 0;
}

Result NfpEmulationService::RequestResetCustomAmiibo()
{
    AmiiboEmulator::ResetCustomAmiibo();
    return 0;
}

Result NfpEmulationService::Toggle()
{
    AmiiboEmulator::Toggle();
    return 0;
}

Result NfpEmulationService::ToggleOnce()
{
    AmiiboEmulator::ToggleOnce();
    return 0;
}

Result NfpEmulationService::Untoggle()
{
    AmiiboEmulator::Untoggle();
    return 0;
}

Result NfpEmulationService::SwapNext()
{
    AmiiboEmulator::SwapNext();
    return 0;
}