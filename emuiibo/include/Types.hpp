
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

#define _VERPARAM_STR(p) #p
#define FORMAT_VERSION _VERPARAM_STR(EMUIIBO_MAJOR) "." _VERPARAM_STR(EMUIIBO_MINOR) "." _VERPARAM_STR(EMUIIBO_MICRO)

// Some QoL macros for zeroing stuff

#define ZERO(ptr, sizeof_tgt) memset(ptr, 0, sizeof(sizeof_tgt))
#define ZERO_PTR(ptr) ZERO(ptr, *ptr)
#define ZERO_NONPTR(nonptr) ZERO(&nonptr, nonptr)

// Logging macro

#define LOG_FMT(...) { \
    std::stringstream strm; \
    strm << "[ emuiibo v" << FORMAT_VERSION << " | " << __PRETTY_FUNCTION__ << " ] " << __VA_ARGS__ << std::endl; \
    FILE *f = fopen((emu::EmuDir + "/emuiibo-log.txt").c_str(), "a+"); \
    if(f) \
    { \
        fprintf(f, "%s", strm.str().c_str()); \
        fclose(f); \
    } \
}

#define DEFINE_RESULT(name, mod, desc) static constexpr Result Result##name = MAKERESULT(mod, desc);

#define LOCK(mtx_name) std::scoped_lock<decltype(mtx_name)> _lock(mtx_name);

#define LOCK_SCOPED(mtx_name, ...) { LOCK(mtx_name); __VA_ARGS__ }