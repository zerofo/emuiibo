
#pragma once
#include <amiibo/amiibo_Formats.hpp>

namespace sys {
    
    enum class EmulationStatus : u32 {
        On,
        Off,
    };

    enum class VirtualAmiiboStatus : u32 {
        Invalid,
        Connected,
        Disconnected
    };

    EmulationStatus GetEmulationStatus();
    void SetEmulationStatus(EmulationStatus status);

    amiibo::VirtualAmiibo &GetActiveVirtualAmiibo();
    bool IsActiveVirtualAmiiboValid();
    void SetActiveVirtualAmiibo(amiibo::VirtualAmiibo amiibo);

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus();
    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status);
    
}