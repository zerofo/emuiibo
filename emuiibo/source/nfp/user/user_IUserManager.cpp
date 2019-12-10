#include "nfp/user/user_IUserManager.hpp"

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

    ams::Result IUserManager::CreateUserInterface(ams::sf::Out<std::shared_ptr<IUser>> out)
    {
        LOG_FMT("Checking status...")
        R_UNLESS(emu::IsStatusOn(), ams::sm::mitm::ResultShouldForwardToSession());
        LOG_FMT("Creating interface...")
        Service outsrv;
        R_TRY(_fwd_CreateUserInterface(&outsrv, this->forward_service.get()));
        LOG_FMT("Created interface... object ID: " << outsrv.object_id)
        const ams::sf::cmif::DomainObjectId target_object_id { serviceGetObjectId(&outsrv) };
        std::shared_ptr<IUser> intf = std::make_shared<IUser>(new Service(outsrv));
        out.SetValue(std::move(intf), target_object_id);
        LOG_FMT("Returning interface...")
        if(emu::IsStatusOnOnce()) emu::SetStatus(emu::EmulationStatus::Off);
        return ams::ResultSuccess();
    }
}