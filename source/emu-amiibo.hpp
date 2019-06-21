
#pragma once
#include <switch.h>
#include <string>
#include <array>

struct AmiiboData
{
    u8 uuid[10];
    u8 pad1[0x4a];
    u8 amiibo_id[0x8];
};

static const u8 DefaultCharInfo[88] =
{
    0xAE, 0x37, 0x34, 0x32, 0xF7, 0x43, 0x4B, 0x39, 0x94, 0xC6, 0xFF, 0xAA, 0x5E, 0xFA, 0x1F, 0xC4,
    0x65, 0x00, 0x6D, 0x00, 0x75, 0x00, 0x69, 0x00, 0x69, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x40, 0x40, 0x00, 0x00, 0x01, 0x04, 0x08,
    0x00, 0x1E, 0x08, 0x01, 0x05, 0x08, 0x04, 0x03, 0x04, 0x02, 0x0C, 0x0E, 0x08, 0x04, 0x03, 0x06,
    0x02, 0x0A, 0x04, 0x04, 0x09, 0x0C, 0x13, 0x04, 0x03, 0x0D, 0x08, 0x00, 0x00, 0x04, 0x0A, 0x00,
    0x08, 0x04, 0x0A, 0x00, 0x04, 0x02, 0x14, 0x00
};

struct AmiiboWriteDate
{
    u16 year;
    u8 month;
    u8 day;
};

struct AmiiboLayout
{
    std::string path;
    std::string name;
    AmiiboWriteDate firstwrite;
    AmiiboWriteDate lastwrite;
    NfpuMiiCharInfo mii;
    u32 appareasize;
    AmiiboData data;
    bool randomuuid;

    NfpuTagInfo ProcessTagInfo();
    NfpuModelInfo ProcessModelInfo();
    NfpuRegisterInfo ProcessRegisterInfo();
    NfpuCommonInfo ProcessCommonInfo();
};

class AmiiboEmulator
{
    public:
        static void Initialize(bool external_call=false);
        static void Toggle();
        static void ToggleOnce();
        static void Untoggle();
        static void SwapNext();
        static u32 GetCount();
        static AmiiboLayout GetCurrentAmiibo();
        static s32 GetCurrentIndex();
        static std::string GetCurrent();
        static std::string GetCurrentName();
        static void SetCustomAmiibo(std::string path);
        static bool IsCustom();
        static void ResetCustomAmiibo();
        static bool ExistsArea(u32 id);
        static void CreateArea(u32 id, u8 *out, u64 size, bool recreate);
        static void ReadArea(u32 id, u64 size, u8 *out);
        static void WriteArea(u32 id, u64 size, u8 *out);
        static u64 GetAreaSize(u32 id);
        static void DumpConsoleMiis();
    private:
        static bool IsDirectory(std::string d);
        static bool IsFile(std::string f);
        static bool IsValidLayout(std::string d);
        static std::string GetAreaName(u32 id);
        static std::string GetNameForIndex(u32 idx);
        static void ConvertToUTF8(char* out, const u16* in, size_t max);
};