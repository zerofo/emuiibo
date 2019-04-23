
#include <switch.h>
#include <string.h>
#include "nfp-shim.h"

static Service g_nfpdbgSrv;
static u64 g_refCnt;

Result nfpDbgInitialize()
{
    atomicIncrement64(&g_refCnt);
    if(serviceIsActive(&g_nfpdbgSrv)) return 0;
    Result rc = smGetService(&g_nfpdbgSrv, "nfp:dbg");
    if(rc == 0) rc = serviceConvertToDomain(&g_nfpdbgSrv);
    return rc;
}

void nfpDbgExit()
{
    if(atomicDecrement64(&g_refCnt) == 0) serviceClose(&g_nfpdbgSrv);
}

Result nfpCreateDebugInterface(NfpDebug* out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw = serviceIpcPrepareHeader(&g_nfpdbgSrv, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 0;

    Result rc = serviceIpcDispatch(&g_nfpdbgSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&g_nfpdbgSrv, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;

        if(R_SUCCEEDED(rc)) serviceCreateSubservice(&out->s, &g_nfpdbgSrv, &r, 0);
    }

    return rc;
}

void nfpDebugClose(NfpDebug* dbg)
{
    serviceClose(&dbg->s);
}

Result nfpDebugInitialize(NfpDebug *dbg, u64 aruid, const u8 *data, size_t size)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddSendBuffer(&c, data, size, BufferType_Normal);
    ipcSendPid(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 aruid;
        u64 zero;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 0;
    raw->aruid = aruid;
    raw->zero = 0;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugFinalize(NfpDebug *dbg)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugListDevices(NfpDebug *dbg, u32 *out, u64 *devices_out, size_t out_size)
{
    IpcCommand c;
    ipcInitialize(&c);
    memset(devices_out, 0, out_size);
    ipcAddRecvStatic(&c, devices_out, out_size, 0);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 2;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u32 count;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0 && out) *out = resp->count;
    }

    return rc;
}

Result nfpDebugStartDetection(NfpDebug *dbg, u64 handle)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 3;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugStopDetection(NfpDebug *dbg, u64 handle)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 4;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugMount(NfpDebug *dbg, u64 handle, u32 type, u32 mount)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
        u32 type;
        u32 mount;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 5;
    raw->handle = handle;
    raw->type = type;
    raw->mount = mount;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugUnmount(NfpDebug *dbg, u64 handle)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 6;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugOpenApplicationArea(NfpDebug *dbg, u64 handle, u32 id, u32 *id_out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
        u32 id;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 7;
    raw->handle = handle;
    raw->id = id;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u32 npad_id;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0) *id_out = resp->npad_id;
    }

    return rc;
}

Result nfpDebugGetApplicationArea(NfpDebug *dbg, u64 handle, u32 *out_size, void *out, size_t size)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddRecvBuffer(&c, out, size, BufferType_Normal);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 8;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u32 size;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0) *out_size = resp->size;
    }

    return rc;
}

Result nfpDebugSetApplicationArea(NfpDebug *dbg, u64 handle, const void *data, size_t size)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddSendBuffer(&c, data, size, BufferType_Normal);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 9;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugFlush(NfpDebug *dbg, u64 handle)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 10;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugRestore(NfpDebug *dbg, u64 handle)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 11;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugCreateApplicationArea(NfpDebug *dbg, u64 handle, u32 id, const void *data, size_t size)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddSendBuffer(&c, data, size, BufferType_Normal);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
        u32 id;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 12;
    raw->handle = handle;
    raw->id = id;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugGetTagInfo(NfpDebug *dbg, u64 handle, NfpuTagInfo *out)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddRecvStatic(&c, out, sizeof(NfpuTagInfo), 0);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 13;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugGetRegisterInfo(NfpDebug *dbg, u64 handle, NfpuRegisterInfo *out)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddRecvStatic(&c, out, sizeof(NfpuRegisterInfo), 0);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 14;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugGetCommonInfo(NfpDebug *dbg, u64 handle, NfpuCommonInfo *out)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddRecvStatic(&c, out, sizeof(NfpuCommonInfo), 0);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 15;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugGetModelInfo(NfpDebug *dbg, u64 handle, NfpuModelInfo *out)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddRecvStatic(&c, out, sizeof(NfpuModelInfo), 0);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 16;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}

Result nfpDebugAttachActivateEvent(NfpDebug *dbg, u64 handle, Handle *out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 17;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0) *out = r.Handles[0];
    }

    return rc;
}

Result nfpDebugAttachDeactivateEvent(NfpDebug *dbg, u64 handle, Handle *out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 18;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0) *out = r.Handles[0];
    }

    return rc;
}

Result nfpDebugGetState(NfpDebug *dbg, u32 *out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 19;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u32 state;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0) *out = resp->state;
    }

    return rc;
}

Result nfpDebugGetDeviceState(NfpDebug *dbg, u64 handle, u32 *out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 20;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u32 state;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0) *out = resp->state;
    }

    return rc;
}

Result nfpDebugGetNpadId(NfpDebug *dbg, u64 handle, u32 *out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 21;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u32 id;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0) *out = resp->id;
    }

    return rc;
}

Result nfpDebugGetApplicationAreaSize(NfpDebug *dbg, u64 handle, u32 *out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 22;
    raw->handle = handle;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u32 size;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0) *out = resp->size;
    }

    return rc;
}

Result nfpDebugAttachAvailabilityChangeEvent(NfpDebug *dbg, Handle *out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 23;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
        if(rc == 0) *out = r.Handles[0];
    }

    return rc;
}

Result nfpDebugRecreateApplicationArea(NfpDebug *dbg, u64 handle, u32 id, const void *data, size_t size)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddSendBuffer(&c, data, size, BufferType_Normal);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 handle;
        u32 id;
    } *raw = serviceIpcPrepareHeader(&dbg->s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 24;
    raw->handle = handle;
    raw->id = id;

    Result rc = serviceIpcDispatch(&dbg->s);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;

        serviceIpcParse(&dbg->s, &r, sizeof(*resp));
        resp = r.Raw;
        rc = resp->result;
    }

    return rc;
}