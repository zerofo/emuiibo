#include "nfp-homebrew-service.hpp"
#include "emu-amiibo.hpp"

extern u32 amiiboIdx;

Result NfpHomebrewService::GetAmiiboCount(Out<u32> out) {
    u32 c = AmiiboEmulator::GetCount();
    out.SetValue(c);
    return 0;
}

Result NfpHomebrewService::GetCurrentAmiibo(Out<u32> idx) {
    idx.SetValue(amiiboIdx);
    return 0;
}