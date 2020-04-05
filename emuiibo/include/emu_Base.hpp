
#pragma once
#include <switch.h>
#include <string>
#include <sstream>
#include <fstream>
#include <mutex>
#include <stratosphere.hpp>
#include <json.hpp>

using i32 = s32;

static inline constexpr Result Success = 0;

using JSON = nlohmann::json;

struct Version {
    u8 major;
    u8 minor;
    u8 micro;
    bool dev_build;
};

static_assert(sizeof(Version) == sizeof(u32), "Invalid Version struct!");

static inline constexpr Version CurrentVersion = { EMUIIBO_MAJOR, EMUIIBO_MINOR, EMUIIBO_MICRO, EMUIIBO_DEV };

namespace consts {

    static inline const std::string EmuDir = "sdmc:/emuiibo";
    static inline const std::string SettingsPath = EmuDir + "/settings.json";
    static inline const std::string LogFilePath = EmuDir + "/emuiibo.log";
    static inline const std::string AmiiboDir = EmuDir + "/amiibo";
    static inline const std::string DumpedMiisDir = EmuDir + "/miis";

}

#define EMU_LOG_FMT(...) { \
    std::stringstream strm; \
    strm << "[ emuiibo v" << EMUIIBO_VERSION << " | " << __PRETTY_FUNCTION__ << " ] " << __VA_ARGS__; \
    auto f = fopen(consts::LogFilePath.c_str(), "a+"); \
    if(f) { \
        fprintf(f, "%s\n", strm.str().c_str()); \
        fclose(f); \
    } \
}

#define EMU_DEFINE_RESULT(name, mod, desc) static constexpr Result Result##name = MAKERESULT(mod, desc);

using Lock = ams::os::RecursiveMutex;

#define EMU_LOCK_SCOPE_WITH(mtx_name) std::scoped_lock lk(mtx_name);