#include "nfp-emu-service.hpp"
#include "emu-amiibo.hpp"

extern FILE *g_logging_file;
extern u32 g_toggleEmulation;

Result NfpEmulationService::GetAmiiboCount(Out<u32> out) {
    u32 c = AmiiboEmulator::GetCount();
    out.SetValue(c);
    return 0;
}

Result NfpEmulationService::GetCurrentAmiibo(Out<u32> idx) {
    s32 cidx = AmiiboEmulator::GetCurrentIndex();
    if(cidx < 0) return LibnxError_NotFound;
    idx.SetValue(cidx);
    return 0;
}

Result NfpEmulationService::RequestCustomAmiibo(InBuffer<char> path) {
    AmiiboEmulator::ForceAmiibo(std::string(path.buffer));
    return 0;
}

Result NfpEmulationService::RequestResetCustomAmiibo() {
    AmiiboEmulator::UnforceAmiibo();
    return 0;
}

Result NfpEmulationService::Toggle() {
    AmiiboEmulator::Toggle();
    return 0;
}

Result NfpEmulationService::ToggleOnce() {
    AmiiboEmulator::ToggleOnce();
    return 0;
}

Result NfpEmulationService::SwapNext() {
    AmiiboEmulator::SwapNext();
    return 0;
}