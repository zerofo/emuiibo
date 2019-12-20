#include "emu/emu_IEmulationService.hpp"
#include "emu/emu_Status.hpp"
#include "emu/emu_Consts.hpp"

namespace emu
{
    #define EMU_CHECK_ON R_UNLESS(emu::IsStatusOn(), result::ResultStatusOff);

    ams::Result IEmulationService::GetCurrentAmiibo(const ams::sf::OutBuffer &path)
    {
        EMU_CHECK_ON

        auto amiibo = GetCurrentLoadedAmiibo();
        R_UNLESS(amiibo.IsValid(), result::ResultNoAmiiboLoaded);

        strcpy((char*)path.GetPointer(), amiibo.Path.c_str());
        
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::SetCustomAmiibo(const ams::sf::InBuffer &path)
    {
        EMU_CHECK_ON

        emu::SetCustomAmiibo(std::string((char*)path.GetPointer()));
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::HasCustomAmiibo(ams::sf::Out<bool> out_has)
    {
        EMU_CHECK_ON

        bool has = emu::HasCustomAmiibo();
        out_has.SetValue(has);
        return ams::ResultSuccess();
    }

    ams::Result IEmulationService::ResetCustomAmiibo()
    {
        EMU_CHECK_ON

        emu::ResetCustomAmiibo();
        return ams::ResultSuccess();
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
        EMU_CHECK_ON

        bool ok = emu::MoveToNextAmiibo();
        R_UNLESS(ok, result::ResultUnableToMove);

        return ams::ResultSuccess();
    }

    void IEmulationService::GetStatus(ams::sf::Out<u32> out_status)
    {
        auto status = emu::GetStatus();
        out_status.SetValue(static_cast<u32>(status));
    }

    ams::Result IEmulationService::Refresh()
    {
        EMU_CHECK_ON
        
        emu::Refresh();
        return ams::ResultSuccess();
    }

    void IEmulationService::GetVersion(ams::sf::Out<Version> out_version)
    {
        out_version.SetValue(CurrentVersion);
    }
}