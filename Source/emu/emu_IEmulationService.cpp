#include <emu/emu_IEmulationService.hpp>
#include <emu/emu_Status.hpp>

namespace emu
{
    Result IEmulationService::GetCurrentAmiibo(OutBuffer<char> path, Out<bool> ok)
    {
        auto amiibo = GetCurrentLoadedAmiibo();
        if(amiibo.IsValid())
        {
            ok.SetValue(true);
            strcpy(path.buffer, amiibo.Path.c_str());
        }
        else
        {
            ok.SetValue(false);
            memset(path.buffer, 0, path.num_elements);
        }
        return 0;
    }

    Result IEmulationService::SetCustomAmiibo(InBuffer<char> path)
    {
        emu::SetCustomAmiibo(std::string(path.buffer));
        return 0;
    }

    Result IEmulationService::HasCustomAmiibo(Out<bool> has)
    {
        bool hasc = emu::HasCustomAmiibo();
        has.SetValue(hasc);
        return 0;
    }

    Result IEmulationService::ResetCustomAmiibo()
    {
        emu::ResetCustomAmiibo();
        return 0;
    }

    Result IEmulationService::SetEmulationOnForever()
    {
        SetStatus(EmulationStatus::OnForever);
        return 0;
    }

    Result IEmulationService::SetEmulationOnOnce()
    {
        SetStatus(EmulationStatus::OnOnce);
        return 0;
    }

    Result IEmulationService::SetEmulationOff()
    {
        SetStatus(EmulationStatus::Off);
        return 0;
    }

    Result IEmulationService::MoveToNextAmiibo(Out<bool> ok)
    {
        bool ook = false;
        if(IsStatusOn()) ook = emu::MoveToNextAmiibo();
        ok.SetValue(ook);
        return 0;
    }

    Result IEmulationService::GetStatus(Out<u32> status)
    {
        auto rawstatus = emu::GetStatus();
        status.SetValue(static_cast<u32>(rawstatus));
        return 0;
    }

    Result IEmulationService::Refresh()
    {
        emu::Refresh();
        return 0;
    }
}