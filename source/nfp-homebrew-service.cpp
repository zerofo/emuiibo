#include "nfp-homebrew-service.hpp"
#include "emu-amiibo.hpp"

extern u32 amiiboIdx;

Result NfpHomebrewService::GetAmiiboCount(Out<u32> out) {
    u32 c = AmiiboEmulator::GetCount();
    out.SetValue(c);
    return 0;
}

Result NfpHomebrewService::GetCurrentAmiibo(Out<u32> idx) {
    s32 cidx = AmiiboEmulator::GetCurrentIndex();
    if(cidx < 0) return LibnxError_NotFound;
    idx.SetValue(cidx);
    return 0;
}

Result NfpHomebrewService::RequestCustomAmiibo(InBuffer<char> path) {
    AmiiboEmulator::ForceAmiibo(std::string(path.buffer));
    return 0;
}

Result NfpHomebrewService::RequestResetCustomAmiibo() {
    AmiiboEmulator::UnforceAmiibo();
    return 0;
}