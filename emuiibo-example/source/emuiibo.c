#include "emuiibo.h"
#include <string.h>

static Service g_emuiibo_nfpemu_srv;

#define EMUIIBO_SRV "nfp:emu"

// Note for developers: the IPC command functions which don't return the Result is because emuiibo will always succeed in those specific commands.

bool emuiiboIsAvailable() {
    SmServiceName srv_name = smEncodeName(EMUIIBO_SRV);
    Handle tmph = 0;
    Result rc = smRegisterService(&tmph, srv_name, false, 1);
    if(R_FAILED(rc)) {
        return true;
    }
    smUnregisterService(srv_name);
    return false;
}

Result emuiiboInitialize() {
    if(serviceIsActive(&g_emuiibo_nfpemu_srv)) {
        return 0;
    }
    return smGetService(&g_emuiibo_nfpemu_srv, EMUIIBO_SRV);
}

void emuiiboExit() {
    if(serviceIsActive(&g_emuiibo_nfpemu_srv)) {
        serviceClose(&g_emuiibo_nfpemu_srv);
    }
}

EmuiiboEmulationStatus emuiiboGetEmulationStatus() {
    u32 out = 0;
    serviceDispatchOut(&g_emuiibo_nfpemu_srv, 0, out);
    return (EmuiiboEmulationStatus)out;
}

void emuiiboSetEmulationStatus(EmuiiboEmulationStatus status) {
    u32 in = (u32)status;
    serviceDispatchIn(&g_emuiibo_nfpemu_srv, 1, in);
}

Result emuiiboGetActiveVirtualAmiibo(EmuiiboVirtualAmiibo *out_amiibo) {
    return serviceDispatch(&g_emuiibo_nfpemu_srv, 2,
        .out_num_objects = 1,
        .out_objects = &out_amiibo->s,
    );
}

void emuiiboResetActiveVirtualAmiibo() {
    serviceDispatch(&g_emuiibo_nfpemu_srv, 3);
}

EmuiiboVirtualAmiiboStatus emuiiboGetActiveVirtualAmiiboStatus() {
    u32 out = 0;
    serviceDispatchOut(&g_emuiibo_nfpemu_srv, 4, out);
    return (EmuiiboVirtualAmiiboStatus)out;
}

void emuiiboSetActiveVirtualAmiiboStatus(EmuiiboVirtualAmiiboStatus status) {
    u32 in = (u32)status;
    serviceDispatchIn(&g_emuiibo_nfpemu_srv, 5, in);
}

u32 emuiiboGetVirtualAmiiboCount() {
    u32 count = 0;
    serviceDispatchOut(&g_emuiibo_nfpemu_srv, 6, count);
    return count;
}

Result emuiiboOpenVirtualAmiibo(u32 idx, EmuiiboVirtualAmiibo *out_amiibo) {
    return serviceDispatchIn(&g_emuiibo_nfpemu_srv, 7, idx,
        .out_num_objects = 1,
        .out_objects = &out_amiibo->s,
    );
}

EmuiiboVersion emuiiboGetVersion() {
    EmuiiboVersion ver;
    memset(&ver, 0, sizeof(ver));
    serviceDispatchOut(&g_emuiibo_nfpemu_srv, 8, ver);
    return ver;
}

void emuiiboVirtualAmiiboSetAsActiveVirtualAmiibo(EmuiiboVirtualAmiibo *amiibo) {
    serviceDispatch(&amiibo->s, 0);
}

void emuiiboVirtualAmiiboGetName(EmuiiboVirtualAmiibo *amiibo, char *out_name, size_t out_name_size) {
    serviceDispatch(&amiibo->s, 1,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { out_name, out_name_size } },
    );
}