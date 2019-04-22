
#pragma once
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Service s;
} NfpDebug;

Result nfpDbgInitialize();
void nfpDbgExit();

Result nfpCreateDebugInterface(NfpDebug* out);
void nfpDebugClose(NfpDebug* dbg);

Result nfpDebugInitialize(NfpDebug *dbg, u64 aruid, const u8 *data, size_t size);
Result nfpDebugFinalize(NfpDebug *dbg);
Result nfpDebugListDevices(NfpDebug *dbg, u32 *out, u64 *devices_out, size_t out_size);
Result nfpDebugStartDetection(NfpDebug *dbg, u64 handle);
Result nfpDebugStopDetection(NfpDebug *dbg, u64 handle);
Result nfpDebugMount(NfpDebug *dbg, u64 handle, u32 type, u32 mount);
Result nfpDebugUnmount(NfpDebug *dbg, u64 handle);
Result nfpDebugOpenApplicationArea(NfpDebug *dbg, u64 handle, u32 id, u32 *id_out);
Result nfpDebugGetApplicationArea(NfpDebug *dbg, u64 handle, u32 *out_size, void *out, size_t size);
Result nfpDebugSetApplicationArea(NfpDebug *dbg, u64 handle, const void *data, size_t size);
Result nfpDebugFlush(NfpDebug *dbg, u64 handle);
Result nfpDebugRestore(NfpDebug *dbg, u64 handle);
Result nfpDebugCreateApplicationArea(NfpDebug *dbg, u64 handle, u32 id, const void *data, size_t size);
Result nfpDebugGetTagInfo(NfpDebug *dbg, u64 handle, NfpuTagInfo *out);
Result nfpDebugGetRegisterInfo(NfpDebug *dbg, u64 handle, NfpuRegisterInfo *out);
Result nfpDebugGetCommonInfo(NfpDebug *dbg, u64 handle, NfpuCommonInfo *out);
Result nfpDebugGetModelInfo(NfpDebug *dbg, u64 handle, NfpuModelInfo *out);
Result nfpDebugAttachActivateEvent(NfpDebug *dbg, u64 handle, Handle *out);
Result nfpDebugAttachDeactivateEvent(NfpDebug *dbg, u64 handle, Handle *out);
Result nfpDebugGetState(NfpDebug *dbg, u32 *out);
Result nfpDebugGetDeviceState(NfpDebug *dbg, u64 handle, u32 *out);
Result nfpDebugGetNpadId(NfpDebug *dbg, u64 handle, u32 *out);
Result nfpDebugGetApplicationAreaSize(NfpDebug *dbg, u64 handle, u32 *out);
Result nfpDebugAttachAvailabilityChangeEvent(NfpDebug *dbg, Handle *out);
Result nfpDebugRecreateApplicationArea(NfpDebug *dbg, u64 handle, u32 id, const void *data, size_t size);

#ifdef __cplusplus
}
#endif