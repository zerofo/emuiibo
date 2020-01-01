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

#include "mii/mii_Service.hpp"

#include "nfp/user/user_IUserManager.hpp"
#include "nfp/sys/sys_ISystemManager.hpp"
#include "emu/emu_IEmulationService.hpp"

#define INNER_HEAP_SIZE 0x10000

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
    ncm::ProgramId CurrentProgramId = { 0x0100000000000352ul };
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

namespace
{
    Result Initialize()
    {
        R_TRY(smInitialize());
        R_TRY(fsInitialize());
        R_TRY(fsdevMountSdmc());
        R_TRY(timeInitialize());
        __libnx_init_time();
        R_TRY(hidInitialize());
        R_TRY(mii::Initialize());
        ams::hos::SetVersionForLibnx();

        return 0;
    }
}

void __appInit(void)
{
    auto rc = Initialize();
    if(R_FAILED(rc)) fatalThrow(rc);
}

void __appExit(void)
{
    mii::Finalize();
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
    auto rc = hidsysInitialize();
    if(R_SUCCEEDED(rc))
    {
        HidsysNotificationLedPattern pattern = {};
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
        rc = hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, UniquePadIds, 2, NULL);
        if(R_SUCCEEDED(rc))
        {
            for(u32 i = 0; i < 2; i++) hidsysSetNotificationLedPattern(&pattern, UniquePadIds[i]);
        }
        hidsysExit();
    }
}

void HOMENotificateOff()
{
    auto rc = hidsysInitialize();
    if(R_SUCCEEDED(rc))
    {
        HidsysNotificationLedPattern pattern = {};
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
        rc = hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, UniquePadIds, 2, NULL);
        if(R_SUCCEEDED(rc))
        {
            for(u32 i = 0; i < 2; i++) hidsysSetNotificationLedPattern(&pattern, UniquePadIds[i]);
        }
        hidsysExit();
    }
}

void HOMENotificateError()
{
    auto rc = hidsysInitialize();
    if(R_SUCCEEDED(rc))
    {
        HidsysNotificationLedPattern pattern = {};
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
        rc = hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, UniquePadIds, 2, NULL);
        if(R_SUCCEEDED(rc))
        {
            for(u32 i = 0; i < 2; i++) hidsysSetNotificationLedPattern(&pattern, UniquePadIds[i]);
        }
        hidsysExit();
    }
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
    auto settings = emu::LoadSettings();
   
    if(settings.combo_enabled)
    {
        ams::os::Thread thread_Input;
        R_ASSERT(thread_Input.Initialize(&InputHandleThread, nullptr, 0x4000, 0x15));
        R_ASSERT(thread_Input.Start());
    }
 
    // Register nfp:user MitM
    R_ASSERT(emuiibo_manager.RegisterMitmServer<nfp::user::IUserManager>(nfp::UserServiceName));

    /*
    // Register nfp:sys MitM - why is this still fucking broken?
    R_ASSERT(emuiibo_manager.RegisterMitmServer<nfp::sys::ISystemManager>(nfp::SystemServiceName));
    */
    
    // Register custom nfp:emu service
    R_ASSERT(emuiibo_manager.RegisterServer<emu::IEmulationService>(emu::EmuServiceName, MaxSessions));
 
    emuiibo_manager.LoopProcess();
 
    return 0;
}