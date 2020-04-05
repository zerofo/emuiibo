
#pragma once
#include <ipc/nfp/sys/sys_ISystem.hpp>

namespace ipc::nfp::sys {
    
    constexpr ams::sm::ServiceName ServiceName = ams::sm::ServiceName::Encode("nfp:sys");

    class ISystemManager : public ICommonManager {

        NFP_USE_CTOR_OF(ICommonManager)

        NFP_COMMON_MANAGER_CREATE_CMD(ISystem)

        NFP_COMMON_MANAGER_BASE
    };

}