
#pragma once
#include <emu_Types.hpp>

namespace result {

    // Official NFC/NFP sysmodule's results

    namespace nfp {

        static constexpr u32 Module = 115;

        EMU_DEFINE_RESULT(DeviceNotFound, Module, 64)
        EMU_DEFINE_RESULT(NeedRestart, Module, 96)
        EMU_DEFINE_RESULT(AreaNeedsToBeCreated, Module, 128)
        EMU_DEFINE_RESULT(AccessIdMismatch, Module, 152)
        EMU_DEFINE_RESULT(AreaAlreadyCreated, Module, 168)

    }
    
    // Our results

    namespace emu {

        static constexpr u32 Module = 352; // Like emuiibo's program ID (0100000000000352)

        EMU_DEFINE_RESULT(NoAmiiboLoaded, Module, 1)
        EMU_DEFINE_RESULT(UnableToMove, Module, 2)
        EMU_DEFINE_RESULT(StatusOff, Module, 3)
        EMU_DEFINE_RESULT(NoMiisFound, Module, 4)
        EMU_DEFINE_RESULT(MiiIndexOOB, Module, 5)

    }

    #define EMU_R_ASSERT(rc) { \
        auto res = (rc); \
        if(R_FAILED(res)) { \
            fatalThrow(static_cast<ams::Result>(res).GetValue()); \
        } \
    }

}