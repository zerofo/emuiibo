#include "emuiibo.h"
#include <iostream>
#include <string>
#include <vector>

#define console(...) { std::cout << __VA_ARGS__ << std::endl; consoleUpdate(nullptr); }
#define console_rc(rc, str) console(str << ": 0x" << std::hex << rc << std::dec)

inline std::string GetVirtualAmiiboName(EmuiiboVirtualAmiibo *amiibo) {
    char amiibo_name[FS_MAX_PATH] = {0};
    emuiiboVirtualAmiiboGetName(amiibo, amiibo_name, sizeof(amiibo_name));
    return amiibo_name;
}

void DoKeyExit() {
    console(std::endl << "Press any key to exit")
    while(true) {
        hidScanInput();
        if(hidKeysDown(CONTROLLER_P1_AUTO)) {
            break;
        }
    }
}

void DoListAmiibos() {
    consoleClear();
    auto count = emuiiboGetVirtualAmiiboCount();
    std::vector<EmuiiboVirtualAmiibo> amiibos;
    for(u32 i = 0; i < count; i++) {
        EmuiiboVirtualAmiibo amiibo;
        if(R_FAILED(emuiiboOpenVirtualAmiibo(i, &amiibo))) {
            break;
        }
        amiibos.push_back(amiibo);
    }
    console("List options:")
    console("")
    console("[X] Move to the next amiibo")
    console("[B] Exit to main menu")
    console("[A] Set the last printed/current amiibo as active")
    console("")
    for(auto &amiibo: amiibos) {
        auto name = GetVirtualAmiiboName(&amiibo);
        console(" - " << name);
        bool move_next = false;
        bool set = false;
        while(appletMainLoop()) {
            hidScanInput();
            auto k = hidKeysDown(CONTROLLER_P1_AUTO);
            if(k & KEY_X) {
                move_next = true;
                break;
            }
            else if(k & KEY_A) {
                set = true;
                break;
            }
            else if(k & KEY_B) {
                break;
            }
        }
        if(!move_next && !set) {
            break;
        }
        if(move_next) {
            continue;
        }
        emuiiboVirtualAmiiboSetAsActiveVirtualAmiibo(&amiibo);
        console("This virtual amiibo was set as active")
        DoKeyExit();
        break;
    }
    for(auto &amiibo: amiibos) {
        serviceClose(&amiibo.s);
    }
}

void DoEnableEmulation() {
    consoleClear();
    emuiiboSetEmulationStatus(EmuiiboEmulationStatus_On);
    console("emuiibo emulation was enabled.")
    DoKeyExit();
}

void DoDisableEmulation() {
    consoleClear();
    emuiiboSetEmulationStatus(EmuiiboEmulationStatus_Off);
    console("emuiibo emulation was disabled.")
    DoKeyExit();
}

void DoResetActive() {
    consoleClear();
    emuiiboResetActiveVirtualAmiibo();
    console("There is no active virtual amiibo now.")
    DoKeyExit();
}

void DoConnect() {
    consoleClear();
    emuiiboSetActiveVirtualAmiiboStatus(EmuiiboVirtualAmiiboStatus_Connected);
    console("The active virtual amiibo was connected.")
    DoKeyExit();
}

void DoDisconnect() {
    consoleClear();
    emuiiboSetActiveVirtualAmiiboStatus(EmuiiboVirtualAmiiboStatus_Disconnected);
    console("The active virtual amiibo was disconnected.")
    DoKeyExit();
}

void PrintMainMenu() {
    consoleClear();
    auto ver = emuiiboGetVersion();
    console("emuiibo v" << (int)ver.major << "." << (int)ver.minor << "." << (int)ver.micro << (emuiiboVersionIsDevBuild(&ver) ? " (dev)" : " (release)"))

    auto status = emuiiboGetEmulationStatus();
    switch(status) {
        case EmuiiboEmulationStatus_On: {
            console("emuiibo emulation is currently enabled.")
            break;
        }
        case EmuiiboEmulationStatus_Off: {
            console("emuiibo emulation is currently disabled.")
            break;
        }
    }

    EmuiiboVirtualAmiibo active_amiibo;
    if(R_SUCCEEDED(emuiiboGetActiveVirtualAmiibo(&active_amiibo))) {
        auto name = GetVirtualAmiiboName(&active_amiibo);
        console("Active virtual amiibo name: " << name)
    }
    else {
        console("There is no active virtual amiibo.")
    }

    console("")
    console("Manager options:")
    console("")
    console("[X] List available amiibos and set one as active")
    console("[Y] Enable emuiibo emulation")
    console("[A] Disable emuiibo emulation")
    console("[B] Reset active virtual amiibo")
    console("[L] Connect the active amiibo")
    console("[R] Disconnect the active amiibo")
    console("[+] Exit")
}

int main() {
    consoleInit(nullptr);
    console(std::endl << "emuiibo sample manager" << std::endl)

    auto ok = emuiiboIsAvailable();
    if(!ok) {
        console("emuiibo is not present or accessible...")
        DoKeyExit();
        consoleExit(nullptr);
        return 0;
    }

    auto rc = emuiiboInitialize();
    if(R_FAILED(rc)) {
        console_rc(rc, "emuiibo services failed to initialize")
        DoKeyExit();
        consoleExit(nullptr);
        return 0;
    }

    PrintMainMenu();
    while(appletMainLoop()) {
        hidScanInput();
        auto k = hidKeysDown(CONTROLLER_P1_AUTO);
        if(k & KEY_X) {
            DoListAmiibos();
            PrintMainMenu();
        }
        else if(k & KEY_Y) {
            DoEnableEmulation();
            PrintMainMenu();
        }
        else if(k & KEY_A) {
            DoDisableEmulation();
            PrintMainMenu();
        }
        else if(k & KEY_B) {
            DoResetActive();
            PrintMainMenu();
        }
        else if(k & KEY_L) {
            DoConnect();
            PrintMainMenu();
        }
        else if(k & KEY_R) {
            DoDisconnect();
            PrintMainMenu();
        }
        else if(k & KEY_PLUS) {
            break;
        }
    }

    emuiiboExit();
    consoleExit(nullptr);

    return 0;
}