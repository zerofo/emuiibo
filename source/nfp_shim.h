/**
 * @file nfp_shim.h
 * @brief Near-Field Proximity Services (nfp) IPC wrapper.
 * @author SciresM
 * @copyright libnx Authors
 */
#pragma once
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Service s;
} NfpUser;

Result nfpCreateUserInterface(Service* s, NfpUser* out);

void nfpUserClose(NfpUser* u);

#ifdef __cplusplus
}
#endif