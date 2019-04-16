
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

bool emuiiboIsPresent();

Result nfpemuInitialize();
void nfpemuExit();

#ifdef __cplusplus
}
#endif