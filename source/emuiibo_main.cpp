/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <atomic>
#include <malloc.h>
#include <switch.h>
#include <vector>
// #include <atmosphere.h>
#include <stratosphere.hpp>

#include "mii-shim.h"
#include "nfpuser_mitm_service.hpp"
#include "nfp-emu-service.hpp"
#include "emu-amiibo.hpp"

extern "C" {
    extern u32 __start__;

    u32 __nx_applet_type = AppletType_None;

    #define INNER_HEAP_SIZE 0x50000
    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];
    
    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}


void __libnx_initheap(void) {
	void*  addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	/* Newlib */
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}

void __appInit(void) {
    Result rc;
    
    rc = smInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
    }
    
    rc = fsInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    }

    rc = fsdevMountSdmc();
    if (R_FAILED(rc)) {
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    }

    rc = hidInitialize();
    if (R_FAILED(rc)) {
            fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));
    }

    rc = miiInitialize();
    if (R_FAILED(rc)) {
            fatalSimple(MAKERESULT(Module_Libnx, 2345));
    }
}

void __appExit(void) {
    /* Cleanup services. */
    miiExit();
    hidExit();
    fsdevUnmountAll();
    fsExit();
    smExit();
}

struct NfpUserManagerOptions {
    static const size_t PointerBufferSize = 0x500;
    static const size_t MaxDomains = 4;
    static const size_t MaxDomainObjects = 0x100;
};

using EmuiiboManager = WaitableManager<NfpUserManagerOptions>;

IEvent* g_eactivate = nullptr;
u32 g_toggleEmulation = 0;
extern HosMutex g_toggleLock;

bool AllKeysDown(std::vector<u64> keys) { // Proper system to get down input of several keys at the same time. TLDR -> one needs to be down at least, and the others down or held (as all would be held).
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

void ComboCheckerThread(void* arg) {
    while(true)
    {
        hidScanInput();
        
        if(AllKeysDown({ KEY_RSTICK, KEY_DUP })) {
            AmiiboEmulator::Toggle();
        }
        else if(AllKeysDown({ KEY_RSTICK, KEY_DRIGHT })) {
            AmiiboEmulator::ToggleOnce();
        }
        else if(AllKeysDown({ KEY_RSTICK, KEY_DDOWN })) {
            AmiiboEmulator::Untoggle();
        }
        else if(AllKeysDown({ KEY_RSTICK, KEY_DLEFT })) {
            AmiiboEmulator::SwapNext();
        }
        
        svcSleepThread(100000000L);
    }
}

FILE *g_logging_file;

int main(int argc, char **argv) {

    remove("sdmc:/emuiibo-new.log");
    g_logging_file = fopen("sdmc:/emuiibo-new.log", "a"); // Log is temporary, release probably won't have it

    AmiiboEmulator::Initialize();

    g_eactivate = CreateWriteOnlySystemEvent<true>();
    
    HosThread comboth;
    comboth.Initialize(&ComboCheckerThread, nullptr, 0x4000, 0x15);
    comboth.Start();
    
    /* Create server manager. */
    auto server_manager = new EmuiiboManager(2);

    // MitM user NFP service...
    AddMitmServerToManager<NfpUserMitmService>(server_manager, "nfp:user", 10);

    // Host custom emulation service...
    server_manager->AddWaitable(new ServiceServer<NfpEmulationService>("nfp:emu", 10));

    /* Loop forever, servicing our services. */
    server_manager->Process();
    
    delete server_manager;
    fclose(g_logging_file);

    return 0;
}
