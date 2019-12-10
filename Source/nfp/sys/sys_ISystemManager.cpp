#include <nfp/sys/sys_ISystemManager.hpp>
#include <emu/emu_Status.hpp>

namespace nfp::sys
{
    ISystemManager::ISystemManager(std::shared_ptr<Service> s, u64 pid, sts::ncm::TitleId tid) : IMitmServiceObject(s, pid, tid)
    {
    }

    void ISystemManager::PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx)
    {
    }

    bool ISystemManager::ShouldMitm(u64 pid, sts::ncm::TitleId tid)
    {
        return emu::IsStatusOn();
    }

    Result ISystemManager::CreateSystemInterface(Out<std::shared_ptr<ISystem>> out)
    {
        if(emu::IsStatusOff()) return ResultAtmosphereMitmShouldForwardToSession;
        std::shared_ptr<ISystem> intf = std::make_shared<ISystem>();
        out.SetValue(std::move(intf));
        if(emu::IsStatusOnOnce()) emu::SetStatus(emu::EmulationStatus::Off);
        return ams::ResultSuccess();
    }
}