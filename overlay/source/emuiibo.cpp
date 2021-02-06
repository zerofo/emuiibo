#include <emuiibo.hpp>

#define EMU_EMUIIBO_SRV "emuiibo"

namespace emu {

    namespace {

        Service g_emuiibo_srv;

        bool smAtmosphereHasService(SmServiceName name) {
            bool has = false;
            serviceDispatchInOut(smGetServiceSession(), 65100, name, has);
            return has;
        }

    }

    bool IsAvailable() {
        return smAtmosphereHasService(smEncodeName(EMU_EMUIIBO_SRV));
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

    void GetVirtualAmiiboDirectory(char *out_path, size_t out_path_size) {
        serviceDispatch(&g_emuiibo_srv, 1,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
            .buffers = { { out_path, out_path_size } },
        );
    }

    EmulationStatus GetEmulationStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_emuiibo_srv, 2, out);
        return static_cast<EmulationStatus>(out);
    }

    void SetEmulationStatus(EmulationStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_emuiibo_srv, 3, in);
    }

    Result GetActiveVirtualAmiibo(VirtualAmiiboData *out_amiibo_data, char *out_path, size_t out_path_size) {
        return serviceDispatchOut(&g_emuiibo_srv, 4, *out_amiibo_data,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_Out
            },
            .buffers = {
                { out_path, out_path_size }
            },
        );
    }

    Result SetActiveVirtualAmiibo(char *path, size_t path_size) {
        return serviceDispatch(&g_emuiibo_srv, 5,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
            .buffers = { { path, path_size } },
        );
    }

    void ResetActiveVirtualAmiibo() {
        serviceDispatch(&g_emuiibo_srv, 6);
    }

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_emuiibo_srv, 7, out);
        return static_cast<VirtualAmiiboStatus>(out);
    }

    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_emuiibo_srv, 8, in);
    }

    void IsApplicationIdIntercepted(u64 app_id, bool *out_intercepted) {
        serviceDispatchInOut(&g_emuiibo_srv, 9, app_id, *out_intercepted);
    }

    Result TryParseVirtualAmiibo(char *path, size_t path_size, VirtualAmiiboData *out_amiibo_data) {
        return serviceDispatchOut(&g_emuiibo_srv, 10, *out_amiibo_data,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_In
            },
            .buffers = {
                { path, path_size }
            },
        );
    }

}