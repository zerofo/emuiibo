#include <emu/emu_IEmulationService.hpp>
#include <emu/emu_Status.hpp>
#include <emu/emu_Consts.hpp>

namespace emu
{
    ams::Result IEmulationService::GetCurrentAmiibo(ams::sf::OutBuffer &path, ams::sf::Out<bool> ok)
    {
        auto amiibo = GetCurrentLoadedAmiibo();
        if(amiibo.IsValid())
        {
            ok.SetValue(true);
            strcpy((char*)path.GetPointer(), amiibo.Path.c_str());
        }
        else
        {
            ok.SetValue(false);
            memset(path.GetPointer(), 0, path.GetSize());
        }
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::SetCustomAmiibo(ams::sf::InBuffer &path)
    {
        emu::SetCustomAmiibo(std::string((char*)path.GetPointer()));
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::HasCustomAmiibo(ams::sf::Out<bool> has)
    {
        bool hasc = emu::HasCustomAmiibo();
        has.SetValue(hasc);
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::ResetCustomAmiibo()
    {
        emu::ResetCustomAmiibo();
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::SetEmulationOnForever()
    {
        SetStatus(EmulationStatus::OnForever);
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::SetEmulationOnOnce()
    {
        SetStatus(EmulationStatus::OnOnce);
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::SetEmulationOff()
    {
        SetStatus(EmulationStatus::Off);
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::MoveToNextAmiibo(ams::sf::Out<bool> ok)
    {
        bool ook = false;
        if(IsStatusOn()) ook = emu::MoveToNextAmiibo();
        ok.SetValue(ook);
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::GetStatus(ams::sf::Out<u32> status)
    {
        auto rawstatus = emu::GetStatus();
        status.SetValue(static_cast<u32>(rawstatus));
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::Refresh()
    {
        emu::Refresh();
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::GetVersion(ams::sf::Out<u32> major, ams::sf::Out<u32> minor, ams::sf::Out<u32> micro)
    {
        major.SetValue(EmuVersion[0]);
        minor.SetValue(EmuVersion[1]);
        micro.SetValue(EmuVersion[2]);
        return ams::ResultSuccess();
    }
}