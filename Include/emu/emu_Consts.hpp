
#pragma once
#include <string>

namespace emu
{
    static const std::string EmuDir = "sdmc:/emuiibo";
    static const std::string AmiiboKeyPath = EmuDir + "/amiibo_key.bin";
    static const std::string AmiiboDir = EmuDir + "/amiibo";
    static const std::string ConsoleMiisDir = EmuDir + "/miis";

    static const u32 EmuVersion[3] = { 0, 3, 0 }; // Major, Minor, Micro
}