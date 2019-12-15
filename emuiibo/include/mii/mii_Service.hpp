
#pragma once
#include <switch.h>

namespace mii
{
    Result Initialize();
    void Finalize();

    // For full random must be 3, 2 and 3
    Result BuildRandom(NfpMiiCharInfo *out, u32 Unk1 = 3, u32 Unk2 = 2, u32 Unk3 = 3);
    Result GetCount(u32 *out_count);
    Result GetCharInfo(u32 idx, NfpMiiCharInfo *out_info);
}