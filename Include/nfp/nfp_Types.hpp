
#pragma once
#include <switch.h>
#include <stratosphere.hpp>

// Some QoL macros for zeroing stuff
#define ZERO(ptr, sizeof_tgt) memset(ptr, 0, sizeof(sizeof_tgt))
#define ZERO_PTR(ptr) ZERO(ptr, *ptr)
#define ZERO_NONPTR(nonptr) ZERO(&nonptr, nonptr)

namespace nfp
{
    struct DeviceHandle
    {
        u32 NpadId;
        u8 Reserved[4];
    } PACKED;

    using TagInfo = NfpuTagInfo;
    using ModelInfo = NfpuModelInfo;
    using RegisterInfo = NfpuRegisterInfo;
    using CommonInfo = NfpuCommonInfo;

    struct AdminInfo
    {
        u8 Data[0x40]; // I guess we have to RE this...
    } PACKED;

    struct GenericManagerOptions
    {
        static const size_t PointerBufferSize = 0x500;
        static const size_t MaxDomains = 4;
        static const size_t MaxDomainObjects = 0x100;
    };

    using ServerManager = WaitableManager<GenericManagerOptions>;

    namespace result
    {
        static constexpr Result ResultNeedRestart = MAKERESULT(115, 96);
        static constexpr Result ResultDeviceNotFound = MAKERESULT(115, 64);
        static constexpr Result ResultAreaNotFound = MAKERESULT(115, 128);
        static constexpr Result ResultAreaAlreadyCreated = MAKERESULT(115, 168);
        static constexpr Result ResultAccessIdMismatch = MAKERESULT(115, 152);
    }
}