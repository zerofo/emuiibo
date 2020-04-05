
#pragma once
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
Source for emuiibo's IPC interface to control virtual amiibo emulation
You can write your own IPC code if you know what you're doing, or use this sources (emuiibo.h and emuiibo.c)

NOTE: this sources are made to work with emuiibo 0.5 - emuiibo's IPC system has almost entirely changed from 0.4 to 0.5, so use these ONLY with 0.5!
This code won't work fine with <0.5 emuiibo.
*/

typedef struct {
    Service s;
} EmuiiboVirtualAmiibo;

typedef enum {
    EmuiiboEmulationStatus_On,
    EmuiiboEmulationStatus_Off,
} EmuiiboEmulationStatus;

typedef enum {
    EmuiiboVirtualAmiiboStatus_Invalid,
    EmuiiboVirtualAmiiboStatus_Connected,
    EmuiiboVirtualAmiiboStatus_Disconnected,  
} EmuiiboVirtualAmiiboStatus;

typedef struct {
    u8 major;
    u8 minor;
    u8 micro;
    u8 dev_build;
} EmuiiboVersion;

NX_CONSTEXPR bool emuiiboVersionIsDevBuild(const EmuiiboVersion *ver) {
    return !!ver->dev_build;
}

NX_CONSTEXPR bool emuiiboVersionMatches(const EmuiiboVersion *ver, u8 major, u8 minor, u8 micro) {
    return (ver->major == major) && (ver->minor == minor) && (ver->micro == micro);
}

typedef enum {
    Module_Emuiibo = 352
} EmuiiboResultModule;

typedef enum {
    EmuiiboError_NoAmiiboLoaded = 1,
    EmuiiboError_UnableToMove = 2,
    EmuiiboError_StatusOff = 3
} EmuiiboResultDescription;

// Note: the service's name is "nfp:emu"

bool emuiiboIsAvailable();

Result emuiiboInitialize();
void emuiiboExit();

EmuiiboEmulationStatus emuiiboGetEmulationStatus();
void emuiiboSetEmulationStatus(EmuiiboEmulationStatus status);

Result emuiiboGetActiveVirtualAmiibo(EmuiiboVirtualAmiibo *out_amiibo);
void emuiiboResetActiveVirtualAmiibo();
EmuiiboVirtualAmiiboStatus emuiiboGetActiveVirtualAmiiboStatus();
void emuiiboSetActiveVirtualAmiiboStatus(EmuiiboVirtualAmiiboStatus status);

u32 emuiiboGetVirtualAmiiboCount();
Result emuiiboOpenVirtualAmiibo(u32 idx, EmuiiboVirtualAmiibo *out_amiibo);

EmuiiboVersion emuiiboGetVersion();

void emuiiboVirtualAmiiboSetAsActiveVirtualAmiibo(EmuiiboVirtualAmiibo *amiibo);
void emuiiboVirtualAmiiboGetName(EmuiiboVirtualAmiibo *amiibo, char *out_name, size_t out_name_size);

#ifdef __cplusplus
}
#endif