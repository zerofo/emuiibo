
#pragma once
#include <stratosphere.hpp>
#include "nfp-shim.h"

class NfpResults
{
    public:
        static constexpr Result ResultNeedRestart = MAKERESULT(115, 96);
        static constexpr Result ResultDeviceNotFound = MAKERESULT(115, 64);
        static constexpr Result ResultAreaNotFound = MAKERESULT(115, 128);
        static constexpr Result ResultAreaAlreadyCreated = MAKERESULT(115, 168);
        static constexpr Result ResultAccessIdMismatch = MAKERESULT(115, 152);
};

struct NfpDeviceHandle
{
    u64 handle;
} PACKED;