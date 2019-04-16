#pragma once
#include <switch.h>
#include <string>
#include <array>

struct AmiiboData {
    u8 uuid[10];
    u8 pad1[0x4a];
    u8 amiibo_id[0x8];
    u8 pad2[0x1C0];
};
static_assert(sizeof(AmiiboData) == 0x21C, "AmiiboData has an invalid size");

class AmiiboEmulator {
    public:
        static void Initialize();
        static u32 GetCount();
        static NfpuTagInfo GetCurrentTagInfo();
        static NfpuModelInfo GetCurrentModelInfo();
        static NfpuRegisterInfo EmulateRegisterInfo();
        static NfpuCommonInfo EmulateCommonInfo();
        static void MoveNext();
        static s32 GetCurrentIndex();
        static void ForceAmiibo(std::string path);
        static bool IsForced();
        static void UnforceAmiibo();
    private:
        static std::string GetNameForIndex(u32 idx);
        static AmiiboData GetRaw();
};