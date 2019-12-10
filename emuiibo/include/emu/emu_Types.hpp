
#pragma once
#include "nfp/nfp_Types.hpp"

namespace emu
{
    enum class EmulationStatus
    {
        OnForever,
        OnOnce,
        Off
    };

    struct RawAmiiboDump
    {
        u8 UUID[0xA];
        u8 Data_0x0FE0F110FFEE[0x6];
        u8 Data_0xA5[0x1];
        u16 UnkCounter;
        u8 Data_0x0_2;
        u8 UnkCryptoRelated[0x40];
        u8 AmiiboIDBlock[0x8];
        // There is more but not interested in that
    } PACKED;

    /*
    
    Amiibo data: (<amiibo> is the amiibo's directory)
    
    - <amiibo>/tag.json:

      - string: uuid (18-length hex, )
      - bool: randomUuid (if this is set the string above has no effect)

    - <amiibo>/model.json:

      - string: amiiboId (8-length hex, full amiibo ID)

    - <amiibo>/register.json:

      - string: name (the amiibo's actual name)
      - string: miiCharInfo (relative path to the mii char-info data, example: "mii-charinfo.bin" being "<amiibo>/mii-charinfo.bin" the file to look for)
      - string: firstWriteDate (YYYY-MM-DD format, example: "2019-06-15")

    - <amiibo>/common.json:

      - string: lastWriteDate (YYYY-MM-DD format, example: "2019-06-15")
      - number: writeCounter (this number shouldn't be change manually, and it increases every time data is written to the amiibo)
      - number: version (defaulted to 0)

    - <amiibo>/areas/:

        Application-specific areas (amiibo's savedata) are written here. Each application has it's own "app ID" (different from title ID) for the savedata.
        
        Format is: 0x<hex-app-id>.bin - example with SSBU: (<amiibo>/areas/)0x34F80200.bin

    */

    struct AmiiboInfos
    {
        nfp::TagInfo Tag;
        nfp::CommonInfo Common;
        nfp::ModelInfo Model;
        nfp::RegisterInfo Register;
        nfp::AdminInfo Admin;
    };

    struct Amiibo
    {
        std::string Path;
        bool RandomizeUUID;
        AmiiboInfos Infos;

        bool IsValid();
        void UpdateWrite();
        std::string MakeAreaName(u32 AreaAppId);
        bool ExistsArea(u32 AreaAppId);
        void CreateArea(u32 AreaAppId, u8 *Data, size_t Size, bool Recreate);
        void WriteArea(u32 AreaAppId, u8 *Data, size_t Size);
        void ReadArea(u32 AreaAppId, u8 *Data, size_t Size);
        size_t GetAreaSize(u32 AreaAppId);
    };

    bool ProcessKeys();
    void ProcessDirectory(std::string Path);
    void DumpConsoleMiis();
    Amiibo ProcessAmiibo(std::string Path);

    // IPC-related types

    constexpr ams::sm::ServiceName EmuServiceName = ams::sm::ServiceName::Encode("nfp:emu");

    namespace result
    {
        static constexpr u32 Module = 352; // Like our program ID (0100000000000352)

        DEFINE_RESULT(NoAmiiboLoaded, Module, 1)
        DEFINE_RESULT(UnableToMove, Module, 2)
        DEFINE_RESULT(StatusOff, Module, 3)
    }
}