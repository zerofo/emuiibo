#include "mii/mii_Service.hpp"
#include "emu/emu_Types.hpp"
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
            u32 in = 0;
            rc = serviceDispatchIn(&miiService, 0, in,
                .out_num_objects = 1,
                .out_objects = &databaseService
            );
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

    Result BuildRandom(NfpMiiCharInfo *out, u32 Unk1, u32 Unk2, u32 Unk3)
    {
        const struct
        {
            u32 unk1;
            u32 unk2;
            u32 unk3;
        } in = { Unk1, Unk2, Unk3 };

        return serviceDispatchInOut(&databaseService, 6, in, *out);
    }

    Result GetCount(u32 *out_count)
    {
        u32 in = 1;

        return serviceDispatchInOut(&databaseService, 2, in, *out_count);
    }

    Result GetCharInfo(u32 idx, NfpMiiCharInfo *out_info)
    {
        u32 mc = 0;
        GetCount(&mc);
        if(mc < 1) return emu::result::ResultNoMiisFound;

        NfpMiiCharInfo *buf = new NfpMiiCharInfo[mc]();
        u32 in = 1;
        u32 outsz = 0;

        auto rc = serviceDispatchInOut(&databaseService, 4, in, outsz, 
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
            .buffers = { { buf, mc * sizeof(NfpMiiCharInfo) } },
        );

        if(R_SUCCEEDED(rc))
        {
            if(idx < outsz) memcpy(out_info, &buf[idx], sizeof(NfpMiiCharInfo));
            else return emu::result::ResultMiiIndexOOB;
        }

        delete[] buf;
        return rc;
    }
}