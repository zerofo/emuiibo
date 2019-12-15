 
#pragma once
#include "nfp/nfp_ICommonObjects.hpp"

namespace nfp::sys
{
    class ISystem : public ICommonInterface
    {
        private:
            enum class CommandId
            {
                NFP_COMMON_IFACE_COMMAND_IDS,

                // Only for nfp:sys!
                Format = 100,
                GetAdminInfo = 101,
                GetRegisterInfo2 = 102,
                SetRegisterInfo = 103,
                DeleteRegisterInfo = 104,
                DeleteApplicationArea = 105,
                ExistsApplicationArea = 106,
            };

        NFP_REGISTER_CTOR(ICommonInterface)

        protected:
            ams::Result Format(DeviceHandle handle);
            ams::Result GetAdminInfo(ams::sf::Out<AdminInfo> out_info, DeviceHandle handle);
            ams::Result GetRegisterInfo2(ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle);
            ams::Result SetRegisterInfo(DeviceHandle handle, const RegisterInfo &info);
            ams::Result DeleteRegisterInfo(DeviceHandle handle);
            ams::Result DeleteApplicationArea(DeviceHandle handle);
            ams::Result ExistsApplicationArea(ams::sf::Out<u8> out_exists, DeviceHandle handle);

        public:
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                NFP_COMMON_IFACE_COMMAND_METAS,

                // Only for nfp:sys!
                MAKE_SERVICE_COMMAND_META(Format),
                MAKE_SERVICE_COMMAND_META(GetAdminInfo),
                MAKE_SERVICE_COMMAND_META(GetRegisterInfo2),
                MAKE_SERVICE_COMMAND_META(SetRegisterInfo),
                MAKE_SERVICE_COMMAND_META(DeleteRegisterInfo),
                MAKE_SERVICE_COMMAND_META(DeleteApplicationArea),
                MAKE_SERVICE_COMMAND_META(ExistsApplicationArea),
            };
    };
}