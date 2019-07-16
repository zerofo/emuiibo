#include <nfp/user/user_IUserManager.hpp>
#include <emu/emu_Status.hpp>

namespace nfp::user
{
    IUserManager::IUserManager(std::shared_ptr<Service> s, u64 pid, sts::ncm::TitleId tid) : IMitmServiceObject(s, pid, tid)
    {
    }

    void IUserManager::PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx)
    {
    }

    bool IUserManager::ShouldMitm(u64 pid, sts::ncm::TitleId tid)
    {
        return emu::IsStatusOn();
    }

    Result IUserManager::CreateUserInterface(Out<std::shared_ptr<IUser>> out)
    {
        if(emu::IsStatusOff()) return ResultAtmosphereMitmShouldForwardToSession;
        std::shared_ptr<IUser> intf = std::make_shared<IUser>();
        out.SetValue(std::move(intf));
        if(emu::IsStatusOnOnce()) emu::SetStatus(emu::EmulationStatus::Off);
        return 0;
    }
}