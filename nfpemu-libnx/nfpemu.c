#include "nfpemu.h"

bool emuiiboIsPresent()
{
    Handle tmph = 0;
    Result rc = smRegisterService(&tmph, "nfp:emu", false, 1);
    if(R_FAILED(rc)) return true;
    smUnregisterService("nfp:emu");
    return false;
}

static Service g_nfpEmuSrv;
static u64 g_refCnt;

Result nfpemuInitialize()
{
    if(!emuiiboIsPresent()) return LibnxError_NotFound;

    atomicIncrement64(&g_refCnt);
    if(serviceIsActive(&g_nfpEmuSrv)) return 0;
    return smGetService(&g_nfpEmuSrv, "nfp:emu");
}

void nfpemuExit()
{
    if(atomicDecrement64(&g_refCnt) == 0) serviceClose(&g_nfpEmuSrv);
}