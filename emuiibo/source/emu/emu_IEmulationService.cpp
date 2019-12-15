#include "emu/emu_IEmulationService.hpp"
#include "emu/emu_Status.hpp"
#include "emu/emu_Consts.hpp"

namespace emu
{
    ams::Result IEmulationService::GetCurrentAmiibo(const ams::sf::OutBuffer &path)
    {
        auto amiibo = GetCurrentLoadedAmiibo();
        R_UNLESS(amiibo.IsValid(), result::ResultNoAmiiboLoaded);

        strcpy((char*)path.GetPointer(), amiibo.Path.c_str());
        
        return ams::ResultSuccess();
    }

    void IEmulationService::SetCustomAmiibo(const ams::sf::InBuffer &path)
    {
        emu::SetCustomAmiibo(std::string((char*)path.GetPointer()));
    }

    void IEmulationService::HasCustomAmiibo(ams::sf::Out<bool> out_has)
    {
        bool has = emu::HasCustomAmiibo();
        out_has.SetValue(has);
    }

    void IEmulationService::ResetCustomAmiibo()
    {
        emu::ResetCustomAmiibo();
    }

    void IEmulationService::SetEmulationOnForever()
    {
        SetStatus(EmulationStatus::OnForever);
    }

    void IEmulationService::SetEmulationOnOnce()
    {
        SetStatus(EmulationStatus::OnOnce);
    }

    void IEmulationService::SetEmulationOff()
    {
        SetStatus(EmulationStatus::Off);
    }

    ams::Result IEmulationService::MoveToNextAmiibo()
    {
        R_UNLESS(IsStatusOn(), result::ResultStatusOff);

        bool ok = emu::MoveToNextAmiibo();
        R_UNLESS(ok, result::ResultUnableToMove);

        return ams::ResultSuccess();
    }

    void IEmulationService::GetStatus(ams::sf::Out<u32> out_status)
    {
        auto status = emu::GetStatus();
        out_status.SetValue(static_cast<u32>(status));
    }

    void IEmulationService::Refresh()
    {
        emu::Refresh();
    }

    void IEmulationService::GetVersion(ams::sf::Out<Version> out_version)
    {
        out_version.SetValue(CurrentVersion);
    }
}