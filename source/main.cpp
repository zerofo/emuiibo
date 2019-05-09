#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <atomic>
#include <malloc.h>
#include <switch.h>
#include <vector>
#include <stratosphere.hpp>

#include "mii-shim.h"
#include "nfpuser-mitm-service.hpp"
#include "nfpsys-mitm-service.hpp"
#include "nfp-emu-service.hpp"
#include "emu-amiibo.hpp"

#define INNER_HEAP_SIZE 0x50000

extern "C"
{
    extern u32 __start__;
    u32 __nx_applet_type = AppletType_None;
    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];
    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}

void __libnx_initheap(void)
{
	void *addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;
	extern char *fake_heap_start;
	extern char *fake_heap_end;
	fake_heap_start = (char*)addr;
	fake_heap_end = (char*)addr + size;
}

void __appInit(void)
{
    Result rc = smInitialize();
    if(R_FAILED(rc)) fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    rc = setsysInitialize();
    if(R_SUCCEEDED(rc))
    {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if(R_SUCCEEDED(rc)) hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }
    
    rc = fsInitialize();
    if(R_FAILED(rc)) fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    rc = fsdevMountSdmc();
    if(R_FAILED(rc)) fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    rc = hidInitialize();
    if(R_FAILED(rc)) fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));
}

void __appExit(void)
{
    hidExit();
    fsdevUnmountAll();
    fsExit();
    smExit();
}

struct NfpUserManagerOptions
{
    static const size_t PointerBufferSize = 0x500;
    static const size_t MaxDomains = 4;
    static const size_t MaxDomainObjects = 0x100;
};

using EmuiiboManager = WaitableManager<NfpUserManagerOptions>;

IEvent* g_eactivate = nullptr;
u32 g_toggleEmulation = 0;
extern HosMutex g_toggleLock;

bool AllKeysDown(std::vector<u64> keys)
{
    u64 kdown = hidKeysDown(CONTROLLER_P1_AUTO);
    u64 kheld = hidKeysHeld(CONTROLLER_P1_AUTO);
    u64 hkd = 0;
    for(u32 i = 0; i < keys.size(); i++)
    {
        if(kdown & keys[i])
        {
            hkd = keys[i];
            break;
        }
    }
    if(hkd == 0) return false;
    bool kk = true;
    for(u32 i = 0; i < keys.size(); i++)
    {
        if(hkd != keys[i])
        {
            if(!(kheld & keys[i]))
            {
                kk = false;
                break;
            }
        }
    }
    return kk;
}

void HOMENotificate()
{
    hidsysInitialize();
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));
    pattern.baseMiniCycleDuration = 0x1;
    pattern.totalMiniCycles = 0x2;
    pattern.totalFullCycles = 0x1;
    pattern.startIntensity = 0x0;
    pattern.miniCycles[0].ledIntensity = 0xf;
    pattern.miniCycles[0].transitionSteps = 0xf;
    pattern.miniCycles[0].finalStepDuration = 0x0;
    pattern.miniCycles[1].ledIntensity = 0x0;
    pattern.miniCycles[1].transitionSteps = 0xf;
    pattern.miniCycles[1].finalStepDuration = 0x0;
    u64 UniquePadIds[2];
    Result rc = hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, UniquePadIds, 2, NULL);
    if(R_SUCCEEDED(rc)) for(u32 i=0; i<2; i++) hidsysSetNotificationLedPattern(&pattern, UniquePadIds[i]);
    hidsysExit();
}

void ComboCheckerThread(void* arg)
{
    while(true)
    {
        hidScanInput();
        if(AllKeysDown({ KEY_RSTICK, KEY_DUP }))
        {
            AmiiboEmulator::Toggle();
            std::scoped_lock<HosMutex> lck(g_toggleLock);
            if(g_toggleEmulation > 0) HOMENotificate();
        }
        else if(AllKeysDown({ KEY_RSTICK, KEY_DRIGHT }))
        {
            AmiiboEmulator::ToggleOnce();
            std::scoped_lock<HosMutex> lck(g_toggleLock);
            if(g_toggleEmulation > 0) HOMENotificate();
        }
        else if(AllKeysDown({ KEY_RSTICK, KEY_DDOWN }))
        {
            std::scoped_lock<HosMutex> lck(g_toggleLock);
            if(g_toggleEmulation > 0) HOMENotificate();
            AmiiboEmulator::Untoggle();
        }
        else if(AllKeysDown({ KEY_RSTICK, KEY_DLEFT }))
        {
            AmiiboEmulator::SwapNext();
            std::scoped_lock<HosMutex> lck(g_toggleLock);
            if(g_toggleEmulation > 0) HOMENotificate();
        }
        svcSleepThread(100000000L);
    }
}


int main()
{
    AmiiboEmulator::Initialize();
    g_eactivate = CreateWriteOnlySystemEvent<true>();
    
    HosThread comboth;
    comboth.Initialize(&ComboCheckerThread, nullptr, 0x4000, 0x15);
    comboth.Start();
    
    auto server_manager = new EmuiiboManager(2);
    AddMitmServerToManager<NfpUserMitmService>(server_manager, "nfp:user", 0x10);
    // AddMitmServerToManager<NfpSystemMitmService>(server_manager, "nfp:sys", 0x10);
    server_manager->AddWaitable(new ServiceServer<NfpEmulationService>("nfp:emu", 0x10));
    server_manager->Process();
    
    delete server_manager;
    return 0;
}
