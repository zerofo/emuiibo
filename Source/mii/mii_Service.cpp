#include <mii/mii_Service.hpp>
#include <cstring>

namespace mii
{
    static Service miiService;
    static Service databaseService;
    static u64 miiRefCount;

    Result Initialize()
    {
        atomicIncrement64(&miiRefCount);
        if(serviceIsActive(&miiService)) return 0;
        Result rc = smGetService(&miiService, "mii:u");
        if(R_SUCCEEDED(rc))
        {
            IpcCommand cmd;
            ipcInitialize(&cmd);

            struct InRaw
            {
                u64 magic;
                u64 cmdid;
                u32 unk1;
            } *raw = (InRaw*)ipcPrepareHeader(&cmd, sizeof(*raw));

            raw->magic = SFCI_MAGIC;
            raw->cmdid = 0;
            raw->unk1 = 0;

            Result rc = serviceIpcDispatch(&miiService);
            if(R_SUCCEEDED(rc))
            {
                IpcParsedCommand pcmd;
                ipcParse(&pcmd);

                struct OutRaw
                {
                    u64 magic;
                    u64 res;
                } *oraw = (OutRaw*)pcmd.Raw;

                rc = oraw->res;
                if(R_SUCCEEDED(rc)) serviceCreateSubservice(&databaseService, &miiService, &pcmd, 0);
            }
        }
        return rc;
    }

    void Finalize()
    {
        if(atomicDecrement64(&miiRefCount) == 0)
        {
            serviceClose(&databaseService);
            serviceClose(&miiService);
        }
    }

    Result BuildRandom(NfpuMiiCharInfo *out, u32 Unk1, u32 Unk2, u32 Unk3)
    {
        IpcCommand cmd;
        ipcInitialize(&cmd);

        struct InRaw
        {
            u64 magic;
            u64 cmdid;
            u32 unk1;
            u32 unk2;
            u32 unk3;
        } *raw = (InRaw*)ipcPrepareHeader(&cmd, sizeof(*raw));

        raw->magic = SFCI_MAGIC;
        raw->cmdid = 6;
        raw->unk1 = Unk1;
        raw->unk2 = Unk2;
        raw->unk3 = Unk3;

        Result rc = serviceIpcDispatch(&databaseService);
        if(R_SUCCEEDED(rc))
        {
            IpcParsedCommand pcmd;
            ipcParse(&pcmd);

            struct OutRaw
            {
                u64 magic;
                u64 res;
                NfpuMiiCharInfo charinfo;
            } *oraw = (OutRaw*)pcmd.Raw;

            rc = oraw->res;
            if(R_SUCCEEDED(rc) && out) memcpy(out, &oraw->charinfo, sizeof(NfpuMiiCharInfo));
        }

        return rc;
    }

    Result GetCount(u32 *out_count)
    {
        IpcCommand cmd;
        ipcInitialize(&cmd);

        struct InRaw
        {
            u64 magic;
            u64 cmdid;
            u32 unk1;
        } *raw = (InRaw*)ipcPrepareHeader(&cmd, sizeof(*raw));

        raw->magic = SFCI_MAGIC;
        raw->cmdid = 2;
        raw->unk1 = 1;

        Result rc = serviceIpcDispatch(&databaseService);
        if(R_SUCCEEDED(rc))
        {
            IpcParsedCommand pcmd;
            ipcParse(&pcmd);

            struct OutRaw
            {
                u64 magic;
                u64 res;
                u32 count;
            } *oraw = (OutRaw*)pcmd.Raw;

            rc = oraw->res;
            if(R_SUCCEEDED(rc) && out_count) *out_count = oraw->count;
        }

        return rc;
    }

    Result GetCharInfo(u32 idx, NfpuMiiCharInfo *out_info)
    {
        u32 mc = 0;
        GetCount(&mc);
        if(mc < 1) return 0xDEAD;
        size_t szbuf = mc * sizeof(NfpuMiiCharInfo);
        NfpuMiiCharInfo *buf = new NfpuMiiCharInfo[mc];

        IpcCommand cmd;
        ipcInitialize(&cmd);
        ipcAddRecvBuffer(&cmd, buf, szbuf, BufferType_Normal);

        struct InRaw
        {
            u64 magic;
            u64 cmdid;
            u32 unk1;
        } *raw = (InRaw*)ipcPrepareHeader(&cmd, sizeof(*raw));

        raw->magic = SFCI_MAGIC;
        raw->cmdid = 4;
        raw->unk1 = 1;

        Result rc = serviceIpcDispatch(&databaseService);
        if(R_SUCCEEDED(rc))
        {
            IpcParsedCommand pcmd;
            ipcParse(&pcmd);

            struct OutRaw
            {
                u64 magic;
                u64 res;
                u32 out;
            } *oraw = (OutRaw*)pcmd.Raw;

            rc = oraw->res;
            if(R_SUCCEEDED(rc) && out_info) memcpy(out_info, &buf[idx], sizeof(NfpuMiiCharInfo));
        }

        delete[] buf;
        return rc;
    }
}