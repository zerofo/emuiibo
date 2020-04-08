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

    Version GetVersion() {
        Version ver = {};
        serviceDispatchOut(&g_emuiibo_srv, 0, ver);
        return ver;
    }

    EmulationStatus GetEmulationStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_emuiibo_srv, 1, out);
        return static_cast<EmulationStatus>(out);
    }

    void SetEmulationStatus(EmulationStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_emuiibo_srv, 2, in);
    }

    Result GetActiveVirtualAmiibo(VirtualAmiiboId *out_amiibo_id, VirtualAmiiboData *out_amiibo_data) {
        return serviceDispatchOut(&g_emuiibo_srv, 3, *out_amiibo_id,
            .buffer_attrs = { SfBufferAttr_FixedSize | SfBufferAttr_HipcPointer | SfBufferAttr_Out },
            .buffers = { { out_amiibo_data, sizeof(VirtualAmiiboData) } },
        );
    }

    Result SetActiveVirtualAmiibo(VirtualAmiiboId *amiibo_id) {
        return serviceDispatchIn(&g_emuiibo_srv, 4, *amiibo_id);
    }

    void ResetActiveVirtualAmiibo() {
        serviceDispatch(&g_emuiibo_srv, 5);
    }

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_emuiibo_srv, 6, out);
        return static_cast<VirtualAmiiboStatus>(out);
    }

    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_emuiibo_srv, 7, in);
    }

    Result ReadNextAvailableVirtualAmiibo(VirtualAmiiboId *out_amiibo_id, VirtualAmiiboData *out_amiibo_data) {
        return serviceDispatchOut(&g_emuiibo_srv, 8, *out_amiibo_id,
            .buffer_attrs = { SfBufferAttr_FixedSize | SfBufferAttr_HipcPointer | SfBufferAttr_Out },
            .buffers = { { out_amiibo_data, sizeof(VirtualAmiiboData) } },
        );
    }

    void ResetAvailableVirtualAmiiboIterator() {
        serviceDispatch(&g_emuiibo_srv, 9);
    }

    void IsApplicationIdIntercepted(u64 app_id, bool *out_intercepted) {
        serviceDispatchInOut(&g_emuiibo_srv, 10, app_id, *out_intercepted);
    }

    void IsCurrentApplicationIdIntercepted(bool *out_intercepted) {
        serviceDispatchOut(&g_emuiibo_srv, 11, *out_intercepted);
    }

}