#include <sys/sys_Emulation.hpp>

namespace sys {

    static EmulationStatus g_emulation_status = EmulationStatus::Off;
    static amiibo::VirtualAmiibo g_virtual_amiibo;
    static VirtualAmiiboStatus g_virtual_amiibo_status = VirtualAmiiboStatus::Invalid;
    static Lock g_emulation_lock;

    EmulationStatus GetEmulationStatus() {
        EMU_LOCK_SCOPE_WITH(g_emulation_lock);
        return g_emulation_status;
    }

    void SetEmulationStatus(EmulationStatus status) {
        EMU_LOCK_SCOPE_WITH(g_emulation_lock);
        g_emulation_status = status;
    }

    amiibo::VirtualAmiibo &GetActiveVirtualAmiibo() {
        EMU_LOCK_SCOPE_WITH(g_emulation_lock);
        return g_virtual_amiibo;
    }

    bool IsActiveVirtualAmiiboValid() {
        EMU_LOCK_SCOPE_WITH(g_emulation_lock);
        return g_virtual_amiibo.IsValid();
    }

    void SetActiveVirtualAmiibo(amiibo::VirtualAmiibo amiibo) {
        EMU_LOCK_SCOPE_WITH(g_emulation_lock);
        g_virtual_amiibo = amiibo;
        SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus::Connected);
    }

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus() {
        EMU_LOCK_SCOPE_WITH(g_emulation_lock);
        if(!IsActiveVirtualAmiiboValid()) {
            g_virtual_amiibo_status = VirtualAmiiboStatus::Invalid;
        }
        return g_virtual_amiibo_status;
    }

    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status) {
        EMU_LOCK_SCOPE_WITH(g_emulation_lock);
        EMU_LOG_FMT("Setting new virtual amiibo status: " << static_cast<u32>(status))
        if(IsActiveVirtualAmiiboValid()) {
            g_virtual_amiibo_status = status;
        }
        else {
            g_virtual_amiibo_status = VirtualAmiiboStatus::Invalid;
        }
    }

}