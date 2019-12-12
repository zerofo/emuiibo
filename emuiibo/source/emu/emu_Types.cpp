#include "emu/emu_Types.hpp"
#include "emu/emu_Consts.hpp"
#include "mii/mii_Service.hpp"
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <cstdio>
#include <ctime>
#include <dirent.h>
#include <fstream>

namespace emu
{
    bool Amiibo::IsValid()
    {
        return ((!Path.empty()) && (Infos.Tag.info.tag_type == 2) && (Infos.Tag.info.uuid_length == 10));
    }

    void Amiibo::UpdateWrite()
    {
        std::ifstream ifs(Path + "/common.json");
        auto jcommon = JSON::parse(ifs);
        auto counter = jcommon["writeCounter"].get<int>();
        if(counter != 0xFFFF)
        {
            counter++;
            jcommon["writeCounter"] = counter;
        }
        auto time = std::time(NULL);
        auto timenow = std::localtime(&time);
        std::stringstream strm;
        strm << std::dec << (timenow->tm_year + 1900) << "-";
        strm << std::dec << std::setw(2) << std::setfill('0') << (int)(timenow->tm_mon + 1) << "-";
        strm << std::dec << std::setw(2) << std::setfill('0') << (int)timenow->tm_mday;
        jcommon["lastWriteDate"] = strm.str();
        ifs.close();
        std::ofstream ofs(Path + "/common.json");
        ofs << std::setw(4) << jcommon;
        ofs.close();
    }

    std::string Amiibo::MakeAreaName(u32 AreaAppId)
    {
        std::stringstream strm;
        strm << Path + "/areas/0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << AreaAppId;
        return strm.str() + ".bin";
    }

    bool Amiibo::ExistsArea(u32 AreaAppId)
    {
        auto area = MakeAreaName(AreaAppId);
        struct stat st;
        return ((stat(area.c_str(), &st) == 0) && (st.st_mode & S_IFREG));
    }

    void Amiibo::CreateArea(u32 AreaAppId, u8 *Data, size_t Size, bool Recreate)
    {
        auto area = MakeAreaName(AreaAppId);
        mkdir((Path + "/areas").c_str(), 777);
        if(Recreate) remove(area.c_str());
        WriteArea(AreaAppId, Data, Size);
    }

    void Amiibo::WriteArea(u32 AreaAppId, u8 *Data, size_t Size)
    {
        auto area = MakeAreaName(AreaAppId);
        FILE *f = fopen(area.c_str(), "wb");
        if(f)
        {
            fwrite(Data, 1, Size, f);
            fclose(f);
            UpdateWrite();
        }
    }

    void Amiibo::ReadArea(u32 AreaAppId, u8 *Data, size_t Size)
    {
        if(ExistsArea(AreaAppId))
        {
            auto area = MakeAreaName(AreaAppId);
            FILE *f = fopen(area.c_str(), "rb");
            if(f)
            {
                fread(Data, 1, Size, f);
                fclose(f);
            }
        }
    }

    size_t Amiibo::GetAreaSize(u32 AreaAppId)
    {
        size_t sz = 0;
        if(ExistsArea(AreaAppId))
        {
            auto area = MakeAreaName(AreaAppId);
            struct stat st;
            stat(area.c_str(), &st);
            sz = st.st_size;
        }
        return sz;
    }

    static const u8 DefaultCharInfo[88] =
    {
        0xAE, 0x37, 0x34, 0x32, 0xF7, 0x43, 0x4B, 0x39, 0x94, 0xC6, 0xFF, 0xAA, 0x5E, 0xFA, 0x1F, 0xC4,
        0x65, 0x00, 0x6D, 0x00, 0x75, 0x00, 0x69, 0x00, 0x69, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x40, 0x40, 0x00, 0x00, 0x01, 0x04, 0x08,
        0x00, 0x1E, 0x08, 0x01, 0x05, 0x08, 0x04, 0x03, 0x04, 0x02, 0x0C, 0x0E, 0x08, 0x04, 0x03, 0x06,
        0x02, 0x0A, 0x04, 0x04, 0x09, 0x0C, 0x13, 0x04, 0x03, 0x0D, 0x08, 0x00, 0x00, 0x04, 0x0A, 0x00,
        0x08, 0x04, 0x0A, 0x00, 0x04, 0x02, 0x14, 0x00
    };

    static void GenerateRandomCharInfo(NfpMiiCharInfo *out)
    {
        auto rc = mii::BuildRandom(out);
        if(R_SUCCEEDED(rc)) utf8_to_utf16(out->mii_name, (const u8*)EMU_DEFAULT_AMIIBO_NAME, strlen(EMU_DEFAULT_AMIIBO_NAME));
    }

    static bool ProcessRawDump(std::string Path)
    {
        auto pathnoext = Path.substr(0, Path.find_last_of("."));
        auto amiiboname = pathnoext.substr(pathnoext.find_last_of("/") + 1);
        auto dir = AmiiboDir + "/" + amiiboname;
        mkdir(dir.c_str(), 777);

        struct stat st;
        if((stat(dir.c_str(), &st) == 0) && (st.st_mode & S_IFDIR)) return false;
        FILE *f = fopen(Path.c_str(), "rb");
        if(f)
        {
            fseek(f, 0, SEEK_END);
            size_t amiibosz = ftell(f);
            rewind(f);
            if(amiibosz < sizeof(RawAmiiboDump))
            {
                fclose(f);
            }
            else
            {
                RawAmiiboDump dump = {};
                fread(&dump, 1, sizeof(RawAmiiboDump), f);
                fclose(f);

                nfp::TagInfo tag = {};
                tag.info.tag_type = 2;
                tag.info.uuid_length = 10;
                memcpy(tag.info.uuid, dump.UUID, 10);
                auto jtag = JSON::object();
                std::stringstream strm;
                for(u32 i = 0; i < 9; i++) strm << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << (int)tag.info.uuid[i];
                jtag["uuid"] = strm.str();
                std::ofstream ofs(dir + "/tag.json");
                ofs << std::setw(4) << jtag;
                ofs.close();
                
                nfp::ModelInfo model = {};
                memcpy(model.info.amiibo_id, dump.AmiiboIDBlock, 8);
                auto jmodel = JSON::object();
                strm.str("");
                strm.clear();
                for(u32 i = 0; i < 8; i++) strm << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << (int)model.info.amiibo_id[i];
                jmodel["amiiboId"] = strm.str();
                ofs = std::ofstream(dir + "/model.json");
                ofs << std::setw(4) << jmodel;
                ofs.close();

                nfp::RegisterInfo reg = {};
                auto name = dir.substr(dir.find_last_of("/") + 1);
                if(name.length() > 10) name = name.substr(0, 10);
                strcpy(reg.info.amiibo_name, name.c_str());

                NfpMiiCharInfo charinfo = {};
                memcpy(&charinfo, DefaultCharInfo, sizeof(charinfo));

                GenerateRandomCharInfo(&charinfo);

                FILE *f = fopen((dir + "/mii-charinfo.bin").c_str(), "wb");
                if(f)
                {
                    fwrite(&charinfo, 1, sizeof(charinfo), f);
                    fclose(f);
                }

                auto time = std::time(NULL);
                auto timenow = std::localtime(&time);

                reg.info.first_write_year = (u16)(timenow->tm_year + 1900);
                reg.info.first_write_month = (u8)(timenow->tm_mon + 1);
                reg.info.first_write_day = (u8)timenow->tm_mday;
                auto jreg = JSON::object();
                jreg["name"] = std::string(reg.info.amiibo_name);
                jreg["miiCharInfo"] = "mii-charinfo.bin";
                strm.str("");
                strm.clear();
                strm << std::dec << reg.info.first_write_year << "-";
                strm << std::dec << std::setw(2) << std::setfill('0') << (int)reg.info.first_write_month << "-";
                strm << std::dec << std::setw(2) << std::setfill('0') << (int)reg.info.first_write_day;
                jreg["firstWriteDate"] = strm.str();
                ofs = std::ofstream(dir + "/register.json");
                ofs << std::setw(4) << jreg;
                ofs.close();
                
                nfp::CommonInfo common = {};
                common.info.last_write_year = timenow->tm_year + 1900;
                common.info.last_write_month = timenow->tm_mon + 1;
                common.info.last_write_day = timenow->tm_mday;
                auto jcommon = JSON::object();
                strm.str("");
                strm.clear();
                strm << std::dec << common.info.last_write_year << "-";
                strm << std::dec << std::setw(2) << std::setfill('0') << (int)common.info.last_write_month << "-";
                strm << std::dec << std::setw(2) << std::setfill('0') << (int)common.info.last_write_day;
                jcommon["lastWriteDate"] = strm.str();
                jcommon["writeCounter"] = (int)common.info.write_counter;
                jcommon["version"] = (int)common.info.version;
                ofs = std::ofstream(dir + "/common.json");
                ofs << std::setw(4) << jcommon;
                ofs.close();
                rename(Path.c_str(), (dir + "/raw-amiibo.bin").c_str());
                return true;
            }
        }
        return false;
    }

    static void ProcessDeprecated(std::string Path)
    {
        auto amiiboname = Path.substr(Path.find_last_of("/") + 1);
        auto outdir = AmiiboDir + "/" + amiiboname;
        mkdir(outdir.c_str(), 777);

        std::ifstream ifs(Path + "/amiibo.json");
        auto old = JSON::parse(ifs);

        FILE *f = fopen((Path + "/amiibo.bin").c_str(), "rb");
        fseek(f, 0, SEEK_END);
        size_t sz = ftell(f);
        rewind(f);
        if(sz < sizeof(RawAmiiboDump))
        {
            fclose(f);
            return;
        }
        RawAmiiboDump dump = {};
        fread(&dump, 1, sizeof(dump), f);
        fclose(f);

        nfp::TagInfo tag = {};
        tag.info.tag_type = 2;
        tag.info.uuid_length = 10;
        memcpy(tag.info.uuid, dump.UUID, 10);
        auto jtag = JSON::object();
        std::stringstream strm;
        for(u32 i = 0; i < 9; i++) strm << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << (int)tag.info.uuid[i];
        bool randomuuid = old.value("randomizeUuid", false);
        if(randomuuid) jtag["randomUuid"] = true;
        else jtag["uuid"] = strm.str();
        std::ofstream ofs(outdir + "/tag.json");
        ofs << std::setw(4) << jtag;
        ofs.close();

        nfp::ModelInfo model = {};
        memcpy(model.info.amiibo_id, dump.AmiiboIDBlock, 8);
        auto jmodel = JSON::object();
        strm.str("");
        strm.clear();
        for(u32 i = 0; i < 8; i++) strm << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << (int)model.info.amiibo_id[i];
        jmodel["amiiboId"] = strm.str();
        ofs = std::ofstream(outdir + "/model.json");
        ofs << std::setw(4) << jmodel;
        ofs.close();

        nfp::RegisterInfo reg = {};
        auto name = old.value("name", amiiboname);
        if(name.length() > 10) name = name.substr(0, 10);
        strcpy(reg.info.amiibo_name, name.c_str());

        NfpMiiCharInfo charinfo = {};

        rename((Path + "/mii.dat").c_str(), (outdir + "/mii-charinfo.bin").c_str());

        auto time = std::time(NULL);
        auto timenow = std::localtime(&time);

        if(old.count("firstWriteDate"))
        {
            reg.info.first_write_year = (u16)old["firstWriteDate"][0];
            reg.info.first_write_month = (u8)old["firstWriteDate"][1];
            reg.info.first_write_day = (u8)old["firstWriteDate"][2];
        }
        else
        {
            reg.info.first_write_year = (u16)(timenow->tm_year + 1900);
            reg.info.first_write_month = (u8)(timenow->tm_mon + 1);
            reg.info.first_write_day = (u8)timenow->tm_mday;
        }
        
        auto jreg = JSON::object();
        jreg["name"] = std::string(reg.info.amiibo_name);
        jreg["miiCharInfo"] = "mii-charinfo.bin";
        strm.str("");
        strm.clear();
        strm << std::dec << reg.info.first_write_year << "-";
        strm << std::dec << std::setw(2) << std::setfill('0') << (int)reg.info.first_write_month << "-";
        strm << std::dec << std::setw(2) << std::setfill('0') << (int)reg.info.first_write_day;
        jreg["firstWriteDate"] = strm.str();
        ofs = std::ofstream(outdir + "/register.json");
        ofs << std::setw(4) << jreg;
        ofs.close();
        
        nfp::CommonInfo common = {};
        common.info.last_write_year = timenow->tm_year + 1900;
        common.info.last_write_month = timenow->tm_mon + 1;
        common.info.last_write_day = timenow->tm_mday;
        auto jcommon = JSON::object();
        strm.str("");
        strm.clear();
        strm << std::dec << common.info.last_write_year << "-";
        strm << std::dec << std::setw(2) << std::setfill('0') << (int)common.info.last_write_month << "-";
        strm << std::dec << std::setw(2) << std::setfill('0') << (int)common.info.last_write_day;
        jcommon["lastWriteDate"] = strm.str();
        jcommon["writeCounter"] = (int)common.info.write_counter;
        jcommon["version"] = (int)common.info.version;
        ofs = std::ofstream(outdir + "/common.json");
        ofs << std::setw(4) << jcommon;
        ofs.close();

        remove((Path + "/amiibo.json").c_str());
        rename((Path + "/amiibo.bin").c_str(), (outdir + "/raw-amiibo.bin").c_str());

        DIR *dp = opendir((Path + "/areas").c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                auto name = std::string(dt->d_name);
                if(name.substr(0, 16) == "ApplicationArea-")
                {
                    auto strappid = name.substr(16);
                    u64 appid = std::stoull(strappid);
                    std::stringstream strm;
                    strm << "0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << appid;
                    auto areaname = strm.str() + ".bin";
                    mkdir((outdir + "/areas").c_str(), 777);
                    rename((Path + "/areas/" + name).c_str(), (outdir + "/areas/" + areaname).c_str());
                }
            }
            closedir(dp);
        }

        ifs.close();

        fsdevDeleteDirectoryRecursively(Path.c_str());
    }

    void ProcessDirectory(std::string Path)
    {
        DIR *dp = opendir(Path.c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                auto name = std::string(dt->d_name);
                auto item = Path + "/" + name;
                auto ext = item.substr(item.find_last_of(".") + 1);
                if(ext == "bin") ProcessRawDump(item);
                else
                {
                    struct stat st;
                    stat((item + "/amiibo.json").c_str(), &st);
                    if(st.st_mode & S_IFREG) ProcessDeprecated(item);
                }
            }
            closedir(dp);
        }
    }

    void DumpConsoleMiis()
    {
        mkdir(ConsoleMiisDir.c_str(), 777);
        u32 miicount = 0;
        auto rc = mii::GetCount(&miicount);
        LOG_FMT("Get Mii count: 0x" << std::hex << rc)
        if(R_SUCCEEDED(rc) && (miicount > 0))
        {
            LOG_FMT("Mii count: " << miicount)
            for(u32 i = 0; i < miicount; i++)
            {
                NfpMiiCharInfo charinfo;
                rc = mii::GetCharInfo(i, &charinfo);
                LOG_FMT("Get Mii charinfo: 0x" << std::hex << rc)
                if(R_SUCCEEDED(rc))
                {
                    char mii_name[0xB] = {0};
                    utf16_to_utf8((u8*)mii_name, (const u16*)charinfo.mii_name, 0x11);
                    LOG_FMT("Got mii name: " << std::hex << mii_name)
                    auto curmiipath = ConsoleMiisDir + "/" + std::to_string(i) + " - " + std::string(mii_name);
                    fsdevDeleteDirectoryRecursively(curmiipath.c_str());
                    mkdir(curmiipath.c_str(), 777);
                    FILE *f = fopen((curmiipath + "/mii-charinfo.bin").c_str(), "wb");
                    if(f)
                    {
                        fwrite(&charinfo, 1, sizeof(charinfo), f);
                        fclose(f);
                    }
                }
            }
        }
    }

    Amiibo ProcessAmiibo(std::string Path)
    {
        Amiibo amiibo = {};
        amiibo.RandomizeUUID = false;
        struct stat st;
        stat(Path.c_str(), &st);
        if(st.st_mode & S_IFDIR)
        {
            std::ifstream ifs(Path + "/tag.json");
            if(ifs.good())
            {
                auto jtag = JSON::parse(ifs);
                std::stringstream strm;
                bool randomuuid = jtag.value("randomUuid", false);
                amiibo.Infos.Tag.info.uuid_length = 10;
                amiibo.Infos.Tag.info.tag_type = 2;
                if(randomuuid)
                {
                    amiibo.RandomizeUUID = true;
                }
                else
                {
                    strm.str("");
                    strm.clear();
                    std::string tmpuuid = jtag["uuid"];
                    for(u32 i = 0; i < tmpuuid.length(); i += 2)
                    {
                        strm << std::hex << tmpuuid.substr(i, 2);
                        int tmpbyte = 0;
                        strm >> tmpbyte;
                        amiibo.Infos.Tag.info.uuid[i / 2] = tmpbyte & 0xFF;
                        strm.str("");
                        strm.clear();
                    }
                }
                ifs.close();
                ifs = std::ifstream(Path + "/model.json");
                if(ifs.good())
                {
                    auto jmodel = JSON::parse(ifs);
                    std::string tmpid = jmodel["amiiboId"];
                    strm.str("");
                    strm.clear();
                    for(u32 i = 0; i < tmpid.length(); i += 2)
                    {
                        strm << std::hex << tmpid.substr(i, 2);
                        int tmpbyte = 0;
                        strm >> tmpbyte;
                        amiibo.Infos.Model.info.amiibo_id[i / 2] = tmpbyte & 0xFF;
                        strm.str("");
                        strm.clear();
                    }
                    ifs.close();
                    ifs = std::ifstream(Path + "/register.json");
                    if(ifs.good())
                    {
                        auto jreg = JSON::parse(ifs);
                        std::string tmpname = jreg["name"];
                        strcpy(amiibo.Infos.Register.info.amiibo_name, tmpname.c_str());
                        std::string miifile = jreg["miiCharInfo"];
                        FILE *f = fopen((Path + "/" + miifile).c_str(), "rb");
                        if(f)
                        {
                            fread(&amiibo.Infos.Register.info.mii, 1, sizeof(NfpMiiCharInfo), f);
                            fclose(f);
                        }
                        else
                        {
                            memcpy(&amiibo.Infos.Register.info.mii, DefaultCharInfo, sizeof(NfpMiiCharInfo));

                            GenerateRandomCharInfo(&amiibo.Infos.Register.info.mii);

                            FILE *f = fopen((Path + "/" + miifile).c_str(), "wb");
                            if(f)
                            {
                                fwrite(&amiibo.Infos.Register.info.mii, 1, sizeof(NfpMiiCharInfo), f);
                                fclose(f);
                            }
                        }
                        
                        std::string tmpdate = jreg["firstWriteDate"];
                        amiibo.Infos.Register.info.first_write_year = (u16)std::stoi(tmpdate.substr(0, 4));
                        amiibo.Infos.Register.info.first_write_month = (u8)std::stoi(tmpdate.substr(5, 2));
                        amiibo.Infos.Register.info.first_write_day = (u8)std::stoi(tmpdate.substr(8, 2));
                        ifs.close();
                        ifs = std::ifstream(Path + "/common.json");
                        if(ifs.good())
                        {
                            auto jcommon = JSON::parse(ifs);
                            tmpdate = jcommon["lastWriteDate"];
                            amiibo.Infos.Common.info.last_write_year = (u16)std::stoi(tmpdate.substr(0, 4));
                            amiibo.Infos.Common.info.last_write_month = (u8)std::stoi(tmpdate.substr(5, 2));
                            amiibo.Infos.Common.info.last_write_day = (u8)std::stoi(tmpdate.substr(8, 2));
                            amiibo.Infos.Common.info.write_counter = (u16)jcommon["writeCounter"];
                            amiibo.Infos.Common.info.version = (u16)jcommon["version"];
                            amiibo.Infos.Common.info.application_area_size = 0xD8;
                            ifs.close();
                            amiibo.Path = Path;
                        }
                    }
                }
            }
        }
        return amiibo;
    }
}