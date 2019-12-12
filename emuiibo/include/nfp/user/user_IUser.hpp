 
#pragma once
#include "nfp/nfp_ICommonObjects.hpp"

namespace nfp::user
{
    class IUser : public ICommonInterface
    {
        private:
            enum class CommandId
            {
                NFP_COMMON_IFACE_COMMAND_IDS,

                // Only for nfp:user!
                OpenApplicationArea = 7,
                GetApplicationArea = 8,
                SetApplicationArea = 9,
                CreateApplicationArea = 12,
                GetApplicationAreaSize = 22,
                RecreateApplicationArea = 24,
            };

        NFP_REGISTER_CTOR(ICommonInterface)

        protected:
            ams::Result OpenApplicationArea(ams::sf::Out<u32> npad_id, DeviceHandle handle, u32 id);
            ams::Result GetApplicationArea(const ams::sf::OutBuffer &data, ams::sf::Out<u32> data_size, DeviceHandle handle);
            ams::Result SetApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle);
            ams::Result CreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, u32 id);
            ams::Result GetApplicationAreaSize(DeviceHandle handle, ams::sf::Out<u32> size);
            ams::Result RecreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, u32 id);

        public:
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                NFP_COMMON_IFACE_COMMAND_METAS,

                // Only for nfp:user!
                MAKE_SERVICE_COMMAND_META(OpenApplicationArea),
                MAKE_SERVICE_COMMAND_META(GetApplicationArea),
                MAKE_SERVICE_COMMAND_META(SetApplicationArea),
                MAKE_SERVICE_COMMAND_META(CreateApplicationArea),
                MAKE_SERVICE_COMMAND_META(GetApplicationAreaSize),
                MAKE_SERVICE_COMMAND_META(RecreateApplicationArea),
            };
    };
}