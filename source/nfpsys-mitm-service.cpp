#include <mutex>
#include <switch.h>
#include <fstream>
#include <iostream>
#include "nfpsys-mitm-service.hpp"

extern HosMutex g_toggleLock;
extern u32 g_toggleEmulation;

void NfpSystemMitmService::PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx)
{
}

bool NfpSystemMitmService::ShouldMitm(u64 pid, u64 tid)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    return (g_toggleEmulation > 0);
}

Result NfpSystemMitmService::CreateSystemInterface(Out<std::shared_ptr<NfpISystem>> out)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return ResultAtmosphereMitmShouldForwardToSession;
    std::shared_ptr<NfpISystem> intf = std::make_shared<NfpISystem>();
    out.SetValue(std::move(intf));
    if(g_toggleEmulation == 2) g_toggleEmulation = 0;
    return 0;
}
