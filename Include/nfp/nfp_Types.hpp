
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <sstream>
#include <cstdio>
#include <iomanip>

// Some QoL macros for zeroing stuff

#define ZERO(ptr, sizeof_tgt) memset(ptr, 0, sizeof(sizeof_tgt))
#define ZERO_PTR(ptr) ZERO(ptr, *ptr)
#define ZERO_NONPTR(nonptr) ZERO(&nonptr, nonptr)

// Logging macro

#define LOG_FMT(...) { \
    std::stringstream strm; \
    strm << "[ emuiibo | " << __PRETTY_FUNCTION__ << " | " << __VA_ARGS__ << std::endl; \
    FILE *f = fopen("sdmc:/fuckoff.txt", "a+"); \
    if(f) \
    { \
        fprintf(f, "%s", strm.str().c_str()); \
        fclose(f); \
    } \
}

namespace nfp
{
    struct DeviceHandle
    {
        u32 NpadId;
        u8 Reserved[4];
    } PACKED;

    #define _NFP_INFO_STRUCT_FOR_IPC(name) \
    struct name##Info : public ams::sf::LargeData \
    { \
        Nfp##name##Info info; \
    }; \
    static_assert(sizeof(name##Info) == sizeof(Nfp##name##Info), "Invalid LargeData struct!");

    _NFP_INFO_STRUCT_FOR_IPC(Tag)
    _NFP_INFO_STRUCT_FOR_IPC(Model)
    _NFP_INFO_STRUCT_FOR_IPC(Common)
    _NFP_INFO_STRUCT_FOR_IPC(Register)

    struct AdminInfo
    {
        u8 Data[0x40]; // I guess we have to RE this...
    } PACKED;

    constexpr ams::sm::ServiceName UserServiceName = ams::sm::ServiceName::Encode("nfp:user");
    constexpr ams::sm::ServiceName SystemServiceName = ams::sm::ServiceName::Encode("nfp:sys");
    constexpr ams::sm::ServiceName EmuServiceName = ams::sm::ServiceName::Encode("nfp:emu");

    #define CUSTOM_SF_MITM_SERVICE_OBJECT_CTOR(cls) cls(std::shared_ptr<::Service> &&s, const ams::sm::MitmProcessInfo &c) : ams::sf::IMitmServiceObject(std::forward<std::shared_ptr<::Service>>(s), c)

    namespace result
    {
        static constexpr Result ResultNeedRestart = MAKERESULT(115, 96);
        static constexpr Result ResultDeviceNotFound = MAKERESULT(115, 64);
        static constexpr Result ResultAreaNotFound = MAKERESULT(115, 128);
        static constexpr Result ResultAreaAlreadyCreated = MAKERESULT(115, 168);
        static constexpr Result ResultAccessIdMismatch = MAKERESULT(115, 152);
    }
}