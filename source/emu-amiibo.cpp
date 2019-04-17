#include "emu-amiibo.hpp"
#include "mii-shim.h"
#include <fstream>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <stratosphere.hpp>
#include <sys/stat.h>

static std::string amiiboDir = "sdmc:/emuiibo";
static std::string fPath = "";
static s32 amiiboIdx = -1;

extern FILE *g_logging_file;
extern HosMutex g_toggleLock;
extern u32 g_toggleEmulation;

void AmiiboEmulator::Initialize() {
    u32 c = GetCount();
    if(c > 0) amiiboIdx = 0;
    else amiiboIdx = -1;
}

void AmiiboEmulator::Toggle() {
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation > 0) g_toggleEmulation = 0; // If toggled, reset
    else g_toggleEmulation = 1; // Keep emulation toggled forever
}

void AmiiboEmulator::ToggleOnce() {
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    g_toggleEmulation = 2; // Just toggle once, then untoggle emulation
}

void AmiiboEmulator::Untoggle() {
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    g_toggleEmulation = 0; // Fully untoggle, no matter it was toggled or not
}

void AmiiboEmulator::SwapNext() {
    if(IsForced()) {
        UnforceAmiibo();
        return;
    }
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation > 0) {
        s32 c = GetCount();
        if((amiiboIdx + 1) < c) amiiboIdx++;
        else amiiboIdx = 0;
    };
}

u32 AmiiboEmulator::GetCount() {
    u32 c = 0;
    DIR *dp = opendir(amiiboDir.c_str());
    if(dp) {
        dirent *dt;
        while(true)
        {
            dt = readdir(dp);
            if(dt == NULL) break;
            std::ifstream ifs(amiiboDir + "/" + std::string(dt->d_name));
            if(ifs.good()) c++;
            ifs.close();
        }
        closedir(dp);
    }
    else {
        amiiboIdx = -1;
    }
    return c;
}

NfpuTagInfo AmiiboEmulator::GetCurrentTagInfo() {
    NfpuTagInfo tag_info;
    memset(&tag_info, 0, sizeof(NfpuTagInfo));
    if(amiiboIdx < 0) return tag_info;

    AmiiboData amiibo = GetRaw();

    memcpy(tag_info.uuid, amiibo.uuid, 10);
    tag_info.uuid_length = 10;
    tag_info.protocol = 0;
    tag_info.tag_type = 2;

    return tag_info;
}

NfpuModelInfo AmiiboEmulator::GetCurrentModelInfo() {

    NfpuModelInfo model_info = {};
    if(amiiboIdx < 0) return model_info;

    AmiiboData amiibo = GetRaw();

    memcpy(model_info.amiibo_id, &amiibo.amiibo_id, 8);

    return model_info;
}

NfpuRegisterInfo AmiiboEmulator::EmulateRegisterInfo() {
    NfpuRegisterInfo reg_info;
    memset(&reg_info, 0, sizeof(NfpuRegisterInfo));

    if(amiiboIdx < 0) return reg_info;

    reg_info.first_write_year = 2019;
    reg_info.first_write_month = 6; // Some fresh
    reg_info.first_write_day = 15;  // memes

    std::string name = (IsForced() ? fPath.substr(fPath.find_last_of("/") + 1) : GetNameForIndex(amiiboIdx));
    std::string noext;
    noext = name.substr(0, name.find_last_of(".")); // Get filename
    if(noext.length() > 10) noext = "Emuiibo"; // Splitting 10+ char names can be ugly

    strcpy(reg_info.amiibo_name, noext.c_str());

    MiiDatabase miidb;
    Result rc = miiGetDatabase(&miidb);
    if(rc == 0) {
        NfpuMiiCharInfo chinfo;
        rc = miiDatabaseGetCharInfo(&miidb, 0, &chinfo);
        if(rc == 0) {
            memcpy(&reg_info.mii, &chinfo, sizeof(NfpuMiiCharInfo));
        }
        miiDatabaseClose(&miidb);
    }

    return reg_info;
}

NfpuCommonInfo AmiiboEmulator::EmulateCommonInfo() {
    NfpuCommonInfo common_info;
    memset(&common_info, 0, sizeof(NfpuCommonInfo));

    common_info.last_write_year = 2019;
    common_info.last_write_month = 6; // Some fresh
    common_info.last_write_day = 15;  // memes
    common_info.application_area_size = 0xd8;

    return common_info;
}

s32 AmiiboEmulator::GetCurrentIndex() {
    return amiiboIdx;
}

void AmiiboEmulator::ForceAmiibo(std::string path) {
    fPath = path;
}

bool AmiiboEmulator::IsForced() {
    return (fPath != "");
}

void AmiiboEmulator::UnforceAmiibo() {
    fPath = "";
}

std::string AmiiboEmulator::GetNameForIndex(u32 idx) {
    std::string name;
    DIR *dp = opendir(amiiboDir.c_str());
    u32 tmpc = 0;
    if(dp) {
        dirent *dt;
        while(true)
        {
            dt = readdir(dp);
            if(dt == NULL) break;
            std::string fname = std::string(dt->d_name);
            std::ifstream ifs(amiiboDir + "/" + fname);
            bool ok = ifs.good();
            ifs.close();
            if(ok)
            {
                if(tmpc == idx) {
                    name = fname;
                    break;
                }
                tmpc++;
            }
        }
        closedir(dp);
    }
    return name;
}

AmiiboData AmiiboEmulator::GetRaw() {
    AmiiboData amiibo = {};
    std::string name = GetNameForIndex(amiiboIdx);
    std::string path = (IsForced() ? fPath : (amiiboDir + "/" + name));
    FILE *f = fopen(path.c_str(), "rb");
    if(f) {
        fread(&amiibo, sizeof(AmiiboData), 1, f);
    }
    fclose(f);
    return amiibo;
}