 
#pragma once
#include <ipc/nfp/nfp_Common.hpp>

namespace ipc::nfp::user {

    namespace impl {

        using namespace ams;

        #define I_USER_INTERFACE_INFO(C, H)                                                        \
            AMS_SF_METHOD_INFO(C, H,  7, ams::Result, OpenApplicationArea,             (DeviceHandle handle, amiibo::AreaId id, ams::sf::Out<u32> out_npad_id))                                             \
            AMS_SF_METHOD_INFO(C, H,  8, ams::Result, GetApplicationArea,             (const ams::sf::OutBuffer &data, ams::sf::Out<u32> data_size, DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  9, ams::Result, SetApplicationArea,             (const ams::sf::InBuffer &data, DeviceHandle handle))                                             \
            AMS_SF_METHOD_INFO(C, H,  12, ams::Result, CreateApplicationArea,             (const ams::sf::InBuffer &data, DeviceHandle handle, amiibo::AreaId id))                                             \
            AMS_SF_METHOD_INFO(C, H,  22, ams::Result, GetApplicationAreaSize,             (DeviceHandle handle, ams::sf::Out<u32> size))                                             \
            AMS_SF_METHOD_INFO(C, H,  24, ams::Result, RecreateApplicationArea,             (const ams::sf::InBuffer &data, DeviceHandle handle, amiibo::AreaId id))
        
        AMS_SF_DEFINE_INTERFACE(IUser, I_USER_INTERFACE_INFO)

    }

    class User final : public CommonInterface {

        private:
            amiibo::AreaId current_opened_area_id;
            bool area_opened;

        public:
            User(Service fwd, u64 app_id) : CommonInterface(fwd, app_id), current_opened_area_id(0), area_opened(false) {}

        public:
            ams::Result OpenApplicationArea(DeviceHandle handle, amiibo::AreaId id, ams::sf::Out<u32> out_npad_id);
            ams::Result GetApplicationArea(const ams::sf::OutBuffer &data, ams::sf::Out<u32> data_size, DeviceHandle handle);
            ams::Result SetApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle);
            ams::Result CreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, amiibo::AreaId id);
            ams::Result GetApplicationAreaSize(DeviceHandle handle, ams::sf::Out<u32> size);
            ams::Result RecreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, amiibo::AreaId id);

    };
    static_assert(nfp::impl::IsICommonInterface<User>);
    static_assert(impl::IsIUser<User>);

}