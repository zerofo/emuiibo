
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <sstream>
#include <cstdio>
#include <iomanip>
#include <json.hpp>
#include "emu/emu_Consts.hpp"

using JSON = nlohmann::json;

struct Version
{
    u8 major;
    u8 minor;
    u8 micro;
    bool dev_build;
};

static_assert(sizeof(Version) == sizeof(u32), "Invalid Version struct!");

static const Version CurrentVersion =
{
    EMUIIBO_MAJOR,
    EMUIIBO_MINOR,
    EMUIIBO_MICRO,
    EMUIIBO_DEV
};

// Logging macro

#define LOG_FMT(...) { \
    std::stringstream strm; \
    strm << "[ emuiibo v" << EMUIIBO_VERSION << " | " << __PRETTY_FUNCTION__ << " ] " << __VA_ARGS__ << std::endl; \
    FILE *f = fopen(emu::LogFilePath.c_str(), "a+"); \
    if(f) \
    { \
        fprintf(f, "%s", strm.str().c_str()); \
        fclose(f); \
    } \
}

#define DEFINE_RESULT(name, mod, desc) static constexpr Result Result##name = MAKERESULT(mod, desc);

#define LOCK(mtx_name) std::scoped_lock<decltype(mtx_name)> _lock(mtx_name);

#define LOCK_SCOPED(mtx_name, ...) { LOCK(mtx_name); __VA_ARGS__ }