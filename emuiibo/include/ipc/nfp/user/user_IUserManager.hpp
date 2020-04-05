
#pragma once
#include <ipc/nfp/user/user_IUser.hpp>

namespace ipc::nfp::user {

    constexpr ams::sm::ServiceName ServiceName = ams::sm::ServiceName::Encode("nfp:user");

    class IUserManager : public ICommonManager {

        NFP_USE_CTOR_OF(ICommonManager)

        NFP_COMMON_MANAGER_CREATE_CMD(IUser)

        NFP_COMMON_MANAGER_BASE

    };

}