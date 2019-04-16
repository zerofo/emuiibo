
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

bool emuiiboIsPresent();

Result nfpemuInitialize();
void nfpemuExit();

Result nfpemuGetAmiiboCount(u32 *out);
Result nfpemuGetCurrentAmiibo(u32 *idx_out);
Result nfpemuRequestCustomAmiibo(const char *path);
Result nfpemuRequestResetCustomAmiibo();

#ifdef __cplusplus
}
#endif