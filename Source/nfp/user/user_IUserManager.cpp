#include <nfp/user/user_IUserManager.hpp>
#include <emu/emu_Status.hpp>
#include <emu/emu_Emulation.hpp>

namespace nfp::user
{
    // Forwards
    
    static Result _fwd_CreateUserInterface(Service *out, Service *base)
    {
        return serviceDispatch(base, 0,
            .out_num_objects = 1,
            .out_objects = out,
        );
    }

    CUSTOM_SF_MITM_SERVICE_OBJECT_CTOR(IUserManager::IUserManager)
    {
        LOG_FMT("Accessed nfp:user with app ID " << std::hex << std::setw(16) << std::setfill('0') << c.program_id.value)
        emu::SetCurrentAppId(c.program_id.value);
    }

    bool IUserManager::ShouldMitm(const ams::sm::MitmProcessInfo &client_info)
    {
        LOG_FMT("Should MitM -> status on: " << std::boolalpha << emu::IsStatusOn())
        return emu::IsStatusOn();
    }

    ams::Result IUserManager::CreateUserInterface(ams::sf::Out<std::shared_ptr<IUser>> out)
    {
        LOG_FMT("Checking status...")
        R_UNLESS(emu::IsStatusOn(), ams::sm::mitm::ResultShouldForwardToSession());
        LOG_FMT("Creating interface...")
        Service outsrv;
        R_TRY(_fwd_CreateUserInterface(&outsrv, this->forward_service.get()));
        LOG_FMT("Created interface... object ID: " << outsrv.object_id)
        const ams::sf::cmif::DomainObjectId target_object_id { serviceGetObjectId(&outsrv) };
        std::shared_ptr<IUser> intf = std::make_shared<IUser>(outsrv);
        out.SetValue(std::move(intf), target_object_id);
        LOG_FMT("Returning interface...")
        if(emu::IsStatusOnOnce()) emu::SetStatus(emu::EmulationStatus::Off);
        return ams::ResultSuccess();
    }
}