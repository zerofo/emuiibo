 
#pragma once
#include <ipc/nfp/nfp_Common.hpp>

namespace ipc::nfp::sys {

    namespace impl {

        using namespace ams;

        #define I_SYSTEM_INTERFACE_INFO(C, H)                                                        \
            AMS_SF_METHOD_INFO(C, H,  100, ams::Result, Format,             (DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  101, ams::Result, GetAdminInfo,             (ams::sf::Out<AdminInfo> out_info, DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  102, ams::Result, GetRegisterInfo2,             (ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  103, ams::Result, SetRegisterInfo,             (DeviceHandle handle, const RegisterInfo &info))                                             \
            AMS_SF_METHOD_INFO(C, H,  104, ams::Result, DeleteRegisterInfo,             (DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  105, ams::Result, DeleteApplicationArea,             (DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  106, ams::Result, ExistsApplicationArea,             (ams::sf::Out<u8> out_exists, DeviceHandle handle))

        AMS_SF_DEFINE_INTERFACE(ISystem, I_SYSTEM_INTERFACE_INFO)

    }

    class System : public CommonInterface {

        public:
            using CommonInterface::CommonInterface;

        public:
            ams::Result Format(DeviceHandle handle);
            ams::Result GetAdminInfo(ams::sf::Out<AdminInfo> out_info, DeviceHandle handle);
            ams::Result GetRegisterInfo2(ams::sf::Out<RegisterInfo> out_info, DeviceHandle handle);
            ams::Result SetRegisterInfo(DeviceHandle handle, const RegisterInfo &info);
            ams::Result DeleteRegisterInfo(DeviceHandle handle);
            ams::Result DeleteApplicationArea(DeviceHandle handle);
            ams::Result ExistsApplicationArea(ams::sf::Out<u8> out_exists, DeviceHandle handle);

    };
    static_assert(nfp::impl::IsICommonInterface<System>);
    static_assert(impl::IsISystem<System>);

}