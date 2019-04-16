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

Result nfpemuGetAmiiboCount(u32 *out)
{
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 0;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
            u32 count;
        } *resp = r.Raw;

        rc = resp->result;
        if(R_SUCCEEDED(rc)) *out = resp->count;
    }

    return rc;
}