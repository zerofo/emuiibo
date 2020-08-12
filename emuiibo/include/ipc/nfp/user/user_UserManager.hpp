
#pragma once
#include <ipc/nfp/user/user_User.hpp>

namespace ipc::nfp::user {

    namespace impl {

        using namespace ams;

        #define I_USER_MANAGER_INTERFACE_INFO(C, H)                                                        \
            AMS_SF_METHOD_INFO(C, H,  0, ams::Result, CreateUserInterface,             (ams::sf::Out<std::shared_ptr<IUser>> out))

        AMS_SF_DEFINE_MITM_INTERFACE(IUserManager, I_USER_MANAGER_INTERFACE_INFO)

    }

    constexpr ams::sm::ServiceName ServiceName = ams::sm::ServiceName::Encode("nfp:user");

    /*
    class UserManager final : public ManagerBase {
        public:
            using ManagerBase::ManagerBase;

        public:
            ams::Result CreateUserInterface(ams::sf::Out<std::shared_ptr<impl::IUser>> out) {
                R_TRY((this->CreateInterfaceImpl<impl::IUser, User>(out)));
                EMU_LOG_FMT("Returning success...");
                return ams::ResultSuccess();
            }
    };
    */
    class UserManager final : public ams::sf::MitmServiceImplBase {

        public:
            using MitmServiceImplBase::MitmServiceImplBase;

            static bool ShouldMitm(const ams::sm::MitmProcessInfo &client_info) {
                return ::sys::GetEmulationStatus() == ::sys::EmulationStatus::On;
            }

            ams::Result CreateUserInterface(ams::sf::Out<std::shared_ptr<impl::IUser>> out) {
                EMU_LOG_FMT("Application ID 0x" << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << this->client_info.program_id.value);
                
                Service outsrv;
                R_UNLESS(::sys::GetEmulationStatus() == ::sys::EmulationStatus::On, ams::sm::mitm::ResultShouldForwardToSession());
                R_TRY(serviceDispatch(this->forward_service.get(), 0,
                    .out_num_objects = 1,
                    .out_objects = &outsrv,
                ));

                const ams::sf::cmif::DomainObjectId object_id { serviceGetObjectId(&outsrv) };
                EMU_LOG_FMT("MakeShared done");
                out.SetValue(ams::sf::MakeShared<impl::IUser, User>(outsrv, this->client_info.program_id.value), object_id);
                EMU_LOG_FMT("Returning success...");
                return ams::ResultSuccess();
            }

    };
    static_assert(impl::IsIUserManager<UserManager>);

}