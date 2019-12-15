
#pragma once
#include "emu/emu_Types.hpp"

namespace emu
{
    void Refresh();
    Amiibo GetCurrentLoadedAmiibo();
    bool MoveToNextAmiibo();
    bool CanMoveNext();
    void SetCustomAmiibo(std::string Path);
    bool HasCustomAmiibo();
    void ResetCustomAmiibo();

    void SetCurrentAppId(u64 Id);
    u64 GetCurrentAppId();
}