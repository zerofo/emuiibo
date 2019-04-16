
#pragma once
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Service s;
} MiiDatabase;

Result miiInitialize();
void miiExit();

Result miiGetDatabase(MiiDatabase *out);
Result miiDatabaseGetCount(MiiDatabase *db, u32 *out_count);
Result miiDatabaseGetCharInfo(MiiDatabase *db, u32 idx, NfpuMiiCharInfo *ch_out);

void miiDatabaseClose(MiiDatabase *db);

#ifdef __cplusplus
}
#endif