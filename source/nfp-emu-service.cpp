#include "nfp-emu-service.hpp"
#include "emu-amiibo.hpp"

extern u32 g_toggleEmulation;
extern HosMutex g_toggleLock;

Result NfpEmulationService::GetAmiibo(OutBuffer<char> path, Out<bool> ok)
{
    auto apath = AmiiboEmulator::GetCurrent();
    if(apath.empty())
    {
        ok.SetValue(false);
        memset(path.buffer, 0, path.num_elements);
    }
    else
    {
        ok.SetValue(true);
        strcpy(path.buffer, apath.c_str());
    }
    return 0;
}

Result NfpEmulationService::SetAmiibo(InBuffer<char> path)
{
    AmiiboEmulator::SetCustomAmiibo(std::string(path.buffer));
    return 0;
}

Result NfpEmulationService::ResetAmiibo()
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

Result NfpEmulationService::GetToggleStatus(Out<u32> status)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    status.SetValue(g_toggleEmulation);
    return 0;
}

Result NfpEmulationService::RescanAmiibos()
{
    AmiiboEmulator::Initialize(true);
    return 0;
}