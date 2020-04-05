#include <emuiibo.hpp>

#define EMU_EMUIIBO_SRV "nfp:emu"

namespace emu {

    static Service g_emuiibo_srv;

    bool IsAvailable() {
        auto srv_name = smEncodeName(EMU_EMUIIBO_SRV);
        Handle tmph = 0;
        auto rc = smRegisterService(&tmph, srv_name, false, 1);
        if(R_FAILED(rc)) {
            return true;
        }
        smUnregisterService(srv_name);
        return false;
    }

    Result Initialize() {
        if(serviceIsActive(&g_emuiibo_srv)) {
            return 0;
        }
        return smGetService(&g_emuiibo_srv, EMU_EMUIIBO_SRV);
    }

    void Exit() {
        serviceClose(&g_emuiibo_srv);
    }

    EmulationStatus GetEmulationStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_emuiibo_srv, 0, out);
        return static_cast<EmulationStatus>(out);
    }

    void SetEmulationStatus(EmulationStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_emuiibo_srv, 1, in);
    }

    Result GetActiveVirtualAmiibo(VirtualAmiibo &amiibo) {
        return serviceDispatch(&g_emuiibo_srv, 2,
            .out_num_objects = 1,
            .out_objects = &amiibo.srv,
        );
    }

    void ResetActiveVirtualAmiibo() {
        serviceDispatch(&g_emuiibo_srv, 3);
    }

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_emuiibo_srv, 4, out);
        return static_cast<VirtualAmiiboStatus>(out);
    }

    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_emuiibo_srv, 5, in);
    }

    u32 GetVirtualAmiiboCount() {
        u32 count = 0;
        serviceDispatchOut(&g_emuiibo_srv, 6, count);
        return count;
    }

    Result OpenVirtualAmiibo(u32 idx, VirtualAmiibo &amiibo) {
        return serviceDispatchIn(&g_emuiibo_srv, 7, idx,
            .out_num_objects = 1,
            .out_objects = &amiibo.srv,
        );
    }

    Version GetVersion() {
        Version ver = {};
        serviceDispatchOut(&g_emuiibo_srv, 8, ver);
        return ver;
    }

}