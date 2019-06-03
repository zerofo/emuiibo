#include "emu-amiibo.hpp"
#include "mii-shim.h"
#include "json.hpp"
#include <fstream>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <stratosphere.hpp>
#include <sys/stat.h>

using json = nlohmann::json;

static std::string g_emuiiboDir = "sdmc:/emuiibo";
static u32 g_curIdx;
static std::string g_curAmiibo;

extern HosMutex g_toggleLock;
extern u32 g_toggleEmulation;

NfpuTagInfo AmiiboLayout::ProcessTagInfo()
{
    NfpuTagInfo tinfo;
    memset(&tinfo, 0, sizeof(NfpuTagInfo));
    memcpy(tinfo.uuid, this->data.uuid, 10);
    if(this->randomuuid) randomGet(tinfo.uuid, 10);
    tinfo.uuid_length = 10;
    tinfo.protocol = 0;
    tinfo.tag_type = 2;
    return tinfo;
}

NfpuModelInfo AmiiboLayout::ProcessModelInfo()
{
    NfpuModelInfo minfo;
    memset(&minfo, 0, sizeof(NfpuModelInfo));
    memcpy(minfo.amiibo_id, this->data.amiibo_id, 8);
    return minfo;
}

NfpuRegisterInfo AmiiboLayout::ProcessRegisterInfo()
{
    NfpuRegisterInfo rinfo;
    memset(&rinfo, 0, sizeof(NfpuRegisterInfo));
    rinfo.first_write_year = this->firstwrite.year;
    rinfo.first_write_month = this->firstwrite.month;
    rinfo.first_write_day = this->firstwrite.day;
    std::string aname = this->name;
    if(aname.length() > 10) aname = aname.substr(0, 10);
    strcpy(rinfo.amiibo_name, aname.c_str());
    memcpy(&rinfo.mii, &this->mii, sizeof(NfpuMiiCharInfo));
    return rinfo;
}

NfpuCommonInfo AmiiboLayout::ProcessCommonInfo()
{
    NfpuCommonInfo cinfo;
    memset(&cinfo, 0, sizeof(NfpuCommonInfo));
    cinfo.last_write_year = this->lastwrite.year;
    cinfo.last_write_month = this->lastwrite.month;
    cinfo.last_write_day = this->lastwrite.day;
    cinfo.application_area_size = this->appareasize;
    return cinfo;
}

void AmiiboEmulator::Initialize(bool external_call)
{
    g_curAmiibo = "";
    g_curIdx = UINT32_MAX;
    if(!external_call)
        DumpConsoleMiis();
    u32 c = GetCount();
    if(c > 0)
    {
        g_curAmiibo = g_emuiiboDir + "/" + GetNameForIndex(0);
        g_curIdx = 0;
    }
}

void AmiiboEmulator::Toggle()
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation > 0) g_toggleEmulation = 0;
    else g_toggleEmulation = 1;
}

void AmiiboEmulator::ToggleOnce()
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    g_toggleEmulation = 2;
}

void AmiiboEmulator::Untoggle()
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    g_toggleEmulation = 0;
}

void AmiiboEmulator::SwapNext()
{
    if(IsCustom())
    {
        u32 c = GetCount();
        if(c > 0)
        {
            g_curAmiibo = g_emuiiboDir + "/" + GetNameForIndex(0);
            g_curIdx = 0;
        }
        return;
    }
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation > 0)
    {
        u32 c = GetCount();
        if(c > 0)
        {
            if((g_curIdx + 1) < c) g_curIdx++;
            else g_curIdx = 0;
            g_curAmiibo = g_emuiiboDir + "/" + GetNameForIndex(g_curIdx);
        }
    }
}

u32 AmiiboEmulator::GetCount()
{
    u32 c = 0;
    DIR *dp = opendir(g_emuiiboDir.c_str());
    if(dp)
    {
        struct dirent *dt;
        while(true)
        {
            dt = readdir(dp);
            if(dt == NULL) break;
            std::string fname = std::string(dt->d_name);
            std::string fdir = g_emuiiboDir + "/" + fname;
            if(IsValidLayout(fdir))
            {
                mkdir((fdir + "/areas").c_str(), 777);
                c++;
            }
            else
            {
                std::string next = fname.substr(fname.find_last_of(".") + 1);
                std::string nname = fname.substr(0, fname.find_last_of("."));
                if(next == "bin")
                {
                    mkdir((g_emuiiboDir + "/" + nname).c_str(), 777);
                    json jdata;
                    jdata["name"] = nname;
                    jdata["firstWriteDate"] = { 2019, 6, 15 };
                    jdata["lastWriteDate"] = { 2019, 6, 15 };
                    jdata["applicationAreaSize"] = 216;
                    std::ofstream ofs(g_emuiiboDir + "/" + nname + "/amiibo.json");
                    ofs << jdata.dump(4);
                    rename((g_emuiiboDir + "/" + fname).c_str(), (g_emuiiboDir + "/" + nname + "/amiibo.bin").c_str());
                    FILE *f = fopen((g_emuiiboDir + "/" + nname + "/mii.dat").c_str(), "wb");
                    fwrite(DefaultCharInfo, 1, 88, f);
                    fclose(f);
                    c++;
                }
            }
        }
    }
    else g_curAmiibo = "";
    closedir(dp);
    return c;
}

AmiiboLayout AmiiboEmulator::GetCurrentAmiibo()
{
    AmiiboLayout lyt;
    std::string lname = g_curAmiibo;
    lyt.path = g_curAmiibo;
    if(g_curAmiibo.empty()) return lyt;
    std::ifstream ifs(lname + "/amiibo.json");
    auto jdata = json::parse(ifs);
    lyt.name = jdata["name"].get<std::string>();
    lyt.firstwrite.year = (u16)jdata["firstWriteDate"][0].get<int>();
    lyt.firstwrite.month = (u8)jdata["firstWriteDate"][1].get<int>();
    lyt.firstwrite.day = (u8)jdata["firstWriteDate"][2].get<int>();
    lyt.lastwrite.year = (u16)jdata["lastWriteDate"][0].get<int>();
    lyt.lastwrite.month = (u8)jdata["lastWriteDate"][1].get<int>();
    lyt.lastwrite.day = (u8)jdata["lastWriteDate"][2].get<int>();
    lyt.appareasize = (u32)jdata["applicationAreaSize"].get<int>();
    lyt.randomuuid = jdata.value<bool>("randomizeUuid", false);
    FILE *miif = fopen((lname + "/mii.dat").c_str(), "rb");
    fread(&lyt.mii, 1, sizeof(NfpuMiiCharInfo), miif);
    fclose(miif);
    FILE *amif = fopen((lname + "/amiibo.bin").c_str(), "rb");
    fread(&lyt.data, 1, sizeof(AmiiboData), amif);
    fclose(amif);
    return lyt;
}

s32 AmiiboEmulator::GetCurrentIndex()
{
    return g_curIdx;
}

std::string AmiiboEmulator::GetCurrent()
{
    return g_curAmiibo;
}

std::string AmiiboEmulator::GetCurrentName()
{
    if(g_curAmiibo.empty()) return g_curAmiibo;
    return g_curAmiibo.substr(g_curAmiibo.find_last_of("/") + 1);
}

void AmiiboEmulator::SetCustomAmiibo(std::string path)
{
    g_curAmiibo = path;
    g_curIdx = UINT32_MAX;
}

bool AmiiboEmulator::IsCustom()
{
    return ((g_curIdx == UINT32_MAX) && (!g_curAmiibo.empty()));
}

void AmiiboEmulator::ResetCustomAmiibo()
{
    if(IsCustom()) SwapNext();
}

bool AmiiboEmulator::ExistsArea(u32 id)
{
    return IsFile(g_curAmiibo + "/areas/" + GetAreaName(id));
}

void AmiiboEmulator::CreateArea(u32 id, u8 *out, u64 size, bool recreate)
{
    mkdir((g_curAmiibo + "/areas").c_str(), 777);
    std::string farea = g_curAmiibo + "/areas/" + GetAreaName(id);
    if(recreate) remove(farea.c_str());
    WriteArea(id, size, out);
}

void AmiiboEmulator::ReadArea(u32 id, u64 size, u8 *out)
{
    std::string farea = g_curAmiibo + "/areas/" + GetAreaName(id);
    FILE *f = fopen(farea.c_str(), "rb");
    fread(out, 1, size, f);
    fclose(f);
}

void AmiiboEmulator::WriteArea(u32 id, u64 size, u8 *out)
{
    std::string farea = g_curAmiibo + "/areas/" + GetAreaName(id);
    FILE *f = fopen(farea.c_str(), "wb");
    fwrite(out, 1, size, f);
    fclose(f);
}

u64 AmiiboEmulator::GetAreaSize(u32 id)
{
    std::string farea = g_curAmiibo + "/areas/" + GetAreaName(id);
    struct stat st;
    stat(farea.c_str(), &st);
    return st.st_size;
}

void AmiiboEmulator::DumpConsoleMiis()
{
    std::string miidir = g_emuiiboDir + "/miis";
    mkdir(miidir.c_str(), 777);
    Result rc = miiInitialize();
    if(rc == 0)
    {
        MiiDatabase miidb;
        rc = miiGetDatabase(&miidb);
        if(rc == 0)
        {
            u32 count = 0;
            miiDatabaseGetCount(&miidb, &count);
            for(u32 i = 0; i < count; i++)
            {
                NfpuMiiCharInfo chinfo;
                rc = miiDatabaseGetCharInfo(&miidb, i, &chinfo);
                if(rc == 0)
                {
                    size_t strsz = sizeof(chinfo.mii_name) / sizeof(u16);
                    char mname[11] = { 0 };
                    ConvertToUTF8(mname, chinfo.mii_name, strsz);
                    std::string miiname = std::to_string(i) + "-" + std::string(mname);
                    remove((miidir + "/" + miiname + ".dat").c_str());
                    FILE *f = fopen((miidir + "/" + miiname + ".dat").c_str(), "wb");
                    fwrite(&chinfo, 1, sizeof(NfpuMiiCharInfo), f);
                    fclose(f);
                }
            }
            miiDatabaseClose(&miidb);
        }
        miiExit();
    }
}

bool AmiiboEmulator::IsDirectory(std::string d)
{
    struct stat st;
    if(stat(d.c_str(), &st) != 0) return false;
    return (st.st_mode & S_IFDIR);
}

bool AmiiboEmulator::IsFile(std::string f)
{
    struct stat st;
    if(stat(f.c_str(), &st) != 0) return false;
    return (st.st_mode & S_IFREG);
}

bool AmiiboEmulator::IsValidLayout(std::string d)
{
    return (IsDirectory(d) && IsFile(d + "/amiibo.json") && IsFile(d + "/amiibo.bin") && IsFile(d + "/mii.dat"));
}

std::string AmiiboEmulator::GetAreaName(u32 id)
{
    return "ApplicationArea-" + std::to_string(id) + ".bin";
}

std::string AmiiboEmulator::GetNameForIndex(u32 idx)
{
    std::string name;
    DIR *dp = opendir(g_emuiiboDir.c_str());
    u32 tmpc = 0;
    if(dp)
    {
        struct dirent *dt;
        while(true)
        {
            dt = readdir(dp);
            if(dt == NULL) break;
            std::string fname = std::string(dt->d_name);
            std::string fdir = g_emuiiboDir + "/" + fname;
            if(IsValidLayout(fdir))
            {
                if(tmpc == idx)
                {
                    name = fname;
                    break;
                }
                tmpc++;
            }
        }
    }
    closedir(dp);
    return name;
}

void AmiiboEmulator::ConvertToUTF8(char* out, const u16* in, size_t max)
{
    if((out == NULL) || (in == NULL)) return;
    out[0] = 0;
    ssize_t units = utf16_to_utf8((u8*)out, in, max);
    if(units < 0) return;
    out[units] = 0;
}