#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <atomic>
#include <malloc.h>
#include <switch.h>
#include <vector>
#include <stratosphere.hpp>

#include "emu/emu_Emulation.hpp"
#include "emu/emu_Status.hpp"

#include "nfp/user/user_IUserManager.hpp"
#include "emu/emu_IEmulationService.hpp"
// #include <nfp/sys/sys_ISystemManager.hpp>

#define INNER_HEAP_SIZE 0x40000

extern "C"
{
    extern u32 __start__;
    u32 __nx_applet_type = AppletType_None;
    u32 __nx_fs_num_sessions = 1;
    u32 __nx_fsdev_direntry_cache_size = 1;

    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];
    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
    void __libnx_init_time(void);
}

namespace ams
{
    ncm::ProgramId CurrentProgramId = { 0x0100000000000352 };
    namespace result
    {
        bool CallFatalOnResultAssertion = true;
    }
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
    if(R_FAILED(rc)) fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    rc = setsysInitialize();
    if(R_SUCCEEDED(rc))
    {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if(R_SUCCEEDED(rc)) hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }
    
    rc = fsInitialize();
    if(R_FAILED(rc)) fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    rc = fsdevMountSdmc();
    if(R_FAILED(rc)) fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    rc = timeInitialize();
    if(R_FAILED(rc)) fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_Time));

    __libnx_init_time();

    rc = hidInitialize();
    if(R_FAILED(rc)) fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

    ams::hos::SetVersionForLibnx();
}

void __appExit(void)
{
    timeExit();
    hidExit();
    fsdevUnmountAll();
    fsExit();
    smExit();
}

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

void HOMENotificateOn()
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

void HOMENotificateOff()
{
    hidsysInitialize();
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));
    pattern.baseMiniCycleDuration = 0x1;
    pattern.totalMiniCycles = 0x4;
    pattern.totalFullCycles = 0x1;
    pattern.startIntensity = 0x4;
    for(u32 i = 0; i < 2; i++)
    {
        pattern.miniCycles[i].ledIntensity = 0x4;
        pattern.miniCycles[i].transitionSteps = 0xf;
        pattern.miniCycles[i].finalStepDuration = 0x1;
    }
    pattern.miniCycles[2].ledIntensity = 0x2;
    pattern.miniCycles[2].transitionSteps = 0xf;
    pattern.miniCycles[2].finalStepDuration = 0x1;
    pattern.miniCycles[3].ledIntensity = 0x0;
    pattern.miniCycles[3].transitionSteps = 0xf;
    pattern.miniCycles[3].finalStepDuration = 0x1;
    u64 UniquePadIds[2];
    Result rc = hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, UniquePadIds, 2, NULL);
    if(R_SUCCEEDED(rc)) for(u32 i=0; i<2; i++) hidsysSetNotificationLedPattern(&pattern, UniquePadIds[i]);
    hidsysExit();
}

void HOMENotificateError()
{
    hidsysInitialize();
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));
    pattern.baseMiniCycleDuration = 0x1;
    pattern.totalMiniCycles = 0x3;
    pattern.totalFullCycles = 0x2;
    pattern.startIntensity = 0x6;
    pattern.miniCycles[0].ledIntensity = 0x6;
    pattern.miniCycles[0].transitionSteps = 0x3;
    pattern.miniCycles[0].finalStepDuration = 0x1;
    pattern.miniCycles[1] = pattern.miniCycles[0];
    pattern.miniCycles[2].ledIntensity = 0x0;
    pattern.miniCycles[2].transitionSteps = 0x3;
    pattern.miniCycles[2].finalStepDuration = 0x1;
    u64 UniquePadIds[2];
    Result rc = hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, UniquePadIds, 2, NULL);
    if(R_SUCCEEDED(rc)) for(u32 i=0; i<2; i++) hidsysSetNotificationLedPattern(&pattern, UniquePadIds[i]);
    hidsysExit();
}

void InputHandleThread(void* arg)
{
    while(true)
    {
        hidScanInput();
        if(AllKeysDown({ KEY_RSTICK, KEY_DUP }))
        {
            if(emu::IsStatusOn())
            {
                emu::SetStatus(emu::EmulationStatus::Off);
                HOMENotificateOff();
            }
            else
            {
                emu::SetStatus(emu::EmulationStatus::OnForever);
                HOMENotificateOn();
            }
        }
        else if(AllKeysDown({ KEY_RSTICK, KEY_DRIGHT }))
        {
            emu::SetStatus(emu::EmulationStatus::OnOnce);
            HOMENotificateOn();
        }
        else if(AllKeysDown({ KEY_RSTICK, KEY_DDOWN }))
        {
            if(emu::IsStatusOn())
            {
                emu::SetStatus(emu::EmulationStatus::Off);
                HOMENotificateOff();
            }
        }
        else if(AllKeysDown({ KEY_RSTICK, KEY_DLEFT }))
        {
            if(emu::IsStatusOn())
            {
                if(emu::MoveToNextAmiibo()) HOMENotificateOn();
                else HOMENotificateError();
            }
            else HOMENotificateError();
        }
        svcSleepThread(100'000'000L);
    }
}

namespace
{
    struct ServerOptions
    {
        static const size_t PointerBufferSize = 0x1000;
        static const size_t MaxDomains = 9;
        static const size_t MaxDomainObjects = 10;
    };

    constexpr size_t MaxServers = 4;
    constexpr size_t MaxSessions = 40;

    ams::sf::hipc::ServerManager<MaxServers, ServerOptions, MaxSessions> emuiibo_manager;
}

int main(int argc, char **argv)
{
    LOG_FMT("Booting up...")
    emu::Refresh();
   
    ams::os::Thread thread_Input;
    R_ASSERT(thread_Input.Initialize(&InputHandleThread, nullptr, 0x4000, 0x15));
    R_ASSERT(thread_Input.Start());
 
    // Register nfp:user MitM
    R_ASSERT(emuiibo_manager.RegisterMitmServer<nfp::user::IUserManager>(nfp::UserServiceName));
    
    // Register custom nfp:emu service
    R_ASSERT(emuiibo_manager.RegisterServer<emu::IEmulationService>(emu::EmuServiceName, MaxSessions));
 
    emuiibo_manager.LoopProcess();
 
    return 0;
}