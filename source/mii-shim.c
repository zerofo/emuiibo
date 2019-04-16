
#include "mii-shim.h"
#include <stdlib.h>
#include <string.h>

static Service g_miiSrv;
static u64 g_refCnt;

Result miiInitialize() {
    atomicIncrement64(&g_refCnt);
    if(serviceIsActive(&g_miiSrv)) return 0;
    return smGetService(&g_miiSrv, "mii:u");
}

void miiExit() {
    if(atomicDecrement64(&g_refCnt) == 0) {
        serviceClose(&g_miiSrv);
    }
}

Result miiGetDatabase(MiiDatabase *out) {
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 Magic;
        u64 CmdId;
        u32 unk;
    } *raw;
    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->Magic = SFCI_MAGIC;
    raw->CmdId = 0;
    raw->unk = 0;
    Result rc = serviceIpcDispatch(&g_miiSrv);
    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);
        struct {
            u64 Magic;
            u64 Result;
        } *resp = r.Raw;
        rc = resp->Result;
        if(rc == 0) serviceCreate(&out->s, r.Handles[0]);
    }
    return rc;
}

Result miiDatabaseGetCount(MiiDatabase *db, u32 *out_count) {
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 Magic;
        u64 CmdId;
        u32 in;
    } *raw;
    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->Magic = SFCI_MAGIC;
    raw->CmdId = 2;
    raw->in = 1; // Why 1? Maybe database index?
    Result rc = serviceIpcDispatch(&db->s);
    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);
        struct {
            u64 Magic;
            u64 Result;
            u32 out;
        } *resp = r.Raw;
        rc = resp->Result;
        if(rc == 0) *out_count = resp->out;
    }
    return rc;
}

Result miiDatabaseGetCharInfo(MiiDatabase *db, u32 idx, NfpuMiiCharInfo *ch_out) {
    u32 mc = 0;
    miiDatabaseGetCount(db, &mc);
    if(mc < 1) return LibnxError_BadInput;
    size_t szbuf = mc * sizeof(NfpuMiiCharInfo);
    NfpuMiiCharInfo *buf = malloc(szbuf);

    IpcCommand c;
    ipcInitialize(&c);
    ipcAddRecvBuffer(&c, buf, szbuf, BufferType_Normal);
    struct {
        u64 Magic;
        u64 CmdId;
        u32 in;
    } *raw;
    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->Magic = SFCI_MAGIC;
    raw->CmdId = 4;
    raw->in = 1; // Again, why 1?
    Result rc = serviceIpcDispatch(&db->s);
    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);
        struct {
            u64 Magic;
            u64 Result;
            u32 out;
        } *resp = r.Raw;
        rc = resp->Result;
    }
    memcpy(ch_out, &buf[idx], sizeof(NfpuMiiCharInfo));
    free(buf);
    return rc;
}

void miiDatabaseClose(MiiDatabase *db) {
    serviceClose(&db->s);
}