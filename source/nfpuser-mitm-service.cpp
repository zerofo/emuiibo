#include <mutex>
#include <switch.h>
#include <fstream>
#include <iostream>
#include "nfpuser-mitm-service.hpp"
#include "nfp-shim.h"

extern HosMutex g_toggleLock;
extern u32 g_toggleEmulation;

void NfpUserMitmService::PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx)
{
}

bool NfpUserMitmService::ShouldMitm(u64 pid, u64 tid)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    return (g_toggleEmulation > 0);
}

Result NfpUserMitmService::CreateUserInterface(Out<std::shared_ptr<NfpIUser>> out)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return ResultAtmosphereMitmShouldForwardToSession;
    std::shared_ptr<NfpIUser> intf = std::make_shared<NfpIUser>();
    out.SetValue(std::move(intf));
    if(g_toggleEmulation == 2) g_toggleEmulation = 0;
    return 0;
}
