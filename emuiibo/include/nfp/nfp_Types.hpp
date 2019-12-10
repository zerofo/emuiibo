
#pragma once
#include "Types.hpp"

namespace nfp
{
    struct DeviceHandle
    {
        u32 npad_id;
        u8 reserved[4];
    };

    static_assert(sizeof(DeviceHandle) == sizeof(u64), "Invalid DeviceHandle struct!");

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

    namespace result
    {
        static constexpr u32 Module = 115;

        DEFINE_RESULT(DeviceNotFound, Module, 64)
        DEFINE_RESULT(NeedRestart, Module, 96)
        DEFINE_RESULT(AreaNotFound, Module, 128)
        DEFINE_RESULT(AccessIdMismatch, Module, 152)
        DEFINE_RESULT(AreaAlreadyCreated, Module, 168)
    }
}