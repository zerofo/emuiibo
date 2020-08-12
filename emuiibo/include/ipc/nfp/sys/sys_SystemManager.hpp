
#pragma once
#include <ipc/nfp/sys/sys_System.hpp>

namespace ipc::nfp::sys {

    namespace impl {

        using namespace ams;

        #define I_SYSTEM_MANAGER_INTERFACE_INFO(C, H)                                                        \
            AMS_SF_METHOD_INFO(C, H,  0, ams::Result, CreateSystemInterface,             (ams::sf::Out<std::shared_ptr<ISystem>> out))

        AMS_SF_DEFINE_MITM_INTERFACE(ISystemManager, I_SYSTEM_MANAGER_INTERFACE_INFO)

    }
    
    constexpr ams::sm::ServiceName ServiceName = ams::sm::ServiceName::Encode("nfp:sys");

    class SystemManager : public ManagerBase {
        public:
            using ManagerBase::ManagerBase;

        public:
            ams::Result CreateSystemInterface(ams::sf::Out<std::shared_ptr<impl::ISystem>> out) {
                R_TRY((this->CreateInterfaceImpl<impl::ISystem, System>(out)));
                return ams::ResultSuccess();
            }
    };
    static_assert(impl::IsISystemManager<SystemManager>);

}