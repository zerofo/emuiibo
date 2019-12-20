
#pragma once
#include <string>

namespace emu
{
    static const std::string EmuDir = "sdmc:/emuiibo";
    static const std::string SettingsPath = EmuDir + "/settings.json";
    static const std::string LogFilePath = EmuDir + "/emuiibo.log";
    static const std::string AmiiboDir = EmuDir + "/amiibo";
    static const std::string ConsoleMiisDir = EmuDir + "/miis";
}