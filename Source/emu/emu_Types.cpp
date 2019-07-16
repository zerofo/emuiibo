#include <emu/emu_Types.hpp>
#include <emu/emu_Consts.hpp>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <cstdio>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <mii/mii_Service.hpp>

namespace emu
{
    bool Amiibo::IsValid()
    {
        return ((!Path.empty()) && (Infos.Tag.tag_type == 2) && (Infos.Tag.uuid_length == 10));
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

    bool ProcessKeys()
    {
        return false;
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

    static bool ProcessRawDump(std::string Path)
    {
        auto dir = Path.substr(0, Path.find_last_of("."));
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
                RawAmiiboDump dump;
                ZERO_NONPTR(dump);
                fread(&dump, 1, sizeof(RawAmiiboDump), f);
                fclose(f);
                mkdir(dir.c_str(), 777);

                nfp::TagInfo tag;
                ZERO_NONPTR(tag);
                tag.tag_type = 2;
                tag.uuid_length = 10;
                memcpy(tag.uuid, dump.UUID, 10);
                auto jtag = JSON::object();
                std::stringstream strm;
                for(u32 i = 0; i < 9; i++) strm << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << (int)tag.uuid[i];
                jtag["uuid"] = strm.str();
                std::ofstream ofs(dir + "/tag.json");
                ofs << std::setw(4) << jtag;
                ofs.close();
                
                nfp::ModelInfo model;
                ZERO_NONPTR(model);
                memcpy(model.amiibo_id, dump.AmiiboIDBlock, 8);
                auto jmodel = JSON::object();
                strm.str("");
                strm.clear();
                for(u32 i = 0; i < 8; i++) strm << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << (int)model.amiibo_id[i];
                jmodel["amiiboId"] = strm.str();
                ofs = std::ofstream(dir + "/model.json");
                ofs << std::setw(4) << jmodel;
                ofs.close();

                nfp::RegisterInfo reg;
                ZERO_NONPTR(reg);
                auto name = dir.substr(dir.find_last_of("/") + 1);
                if(name.length() > 10) name = name.substr(0, 10);
                strcpy(reg.amiibo_name, name.c_str());

                NfpuMiiCharInfo charinfo;
                ZERO_NONPTR(charinfo);
                memcpy(&charinfo, DefaultCharInfo, sizeof(NfpuMiiCharInfo));

                auto rc = mii::Initialize();
                if(rc == 0)
                {
                    mii::BuildRandom(&charinfo);
                    // Since mii service generates random miis with name "no name", change to "emuiibo"
                    const char *name = "emuiibo";
                    utf8_to_utf16(charinfo.mii_name, (const u8*)name, strlen(name));
                    mii::Finalize();
                }

                FILE *f = fopen((dir + "/mii-charinfo.bin").c_str(), "wb");
                if(f)
                {
                    fwrite(&charinfo, 1, sizeof(NfpuMiiCharInfo), f);
                    fclose(f);
                }

                auto time = std::time(NULL);
                auto timenow = std::localtime(&time);

                reg.first_write_year = (u16)(timenow->tm_year + 1900);
                reg.first_write_month = (u8)(timenow->tm_mon + 1);
                reg.first_write_day = (u8)timenow->tm_mday;
                auto jreg = JSON::object();
                jreg["name"] = std::string(reg.amiibo_name);
                jreg["miiCharInfo"] = "mii-charinfo.bin";
                strm.str("");
                strm.clear();
                strm << std::dec << reg.first_write_year << "-";
                strm << std::dec << std::setw(2) << std::setfill('0') << (int)reg.first_write_month << "-";
                strm << std::dec << std::setw(2) << std::setfill('0') << (int)reg.first_write_day;
                jreg["firstWriteDate"] = strm.str();
                ofs = std::ofstream(dir + "/register.json");
                ofs << std::setw(4) << jreg;
                ofs.close();
                
                nfp::CommonInfo common;
                ZERO_NONPTR(common);
                common.last_write_year = timenow->tm_year + 1900;
                common.last_write_month = timenow->tm_mon + 1;
                common.last_write_day = timenow->tm_mday;
                auto jcommon = JSON::object();
                strm.str("");
                strm.clear();
                strm << std::dec << common.last_write_year << "-";
                strm << std::dec << std::setw(2) << std::setfill('0') << (int)common.last_write_month << "-";
                strm << std::dec << std::setw(2) << std::setfill('0') << (int)common.last_write_day;
                jcommon["lastWriteDate"] = strm.str();
                jcommon["writeCounter"] = (int)common.write_counter;
                jcommon["version"] = (int)common.version;
                ofs = std::ofstream(dir + "/common.json");
                ofs << std::setw(4) << jcommon;
                ofs.close();
                return true;
            }
        }
        return false;
    }

    void ProcessSingleDumps()
    {
        DIR *dp = opendir(AmiiboDir.c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                auto name = std::string(dt->d_name);
                auto item = AmiiboDir + "/" + name;
                auto ext = item.substr(item.find_last_of(".") + 1);
                if(ext == "bin") ProcessRawDump(item);
            }
            closedir(dp);
        }
    }

    Amiibo ProcessAmiibo(std::string Path)
    {
        Amiibo amiibo;
        amiibo.RandomizeUUID = false;
        ZERO_NONPTR(amiibo.Infos);
        struct stat st;
        stat(Path.c_str(), &st);
        auto ext = Path.substr(Path.find_last_of(".") + 1);
        if(st.st_mode & S_IFDIR)
        {
            std::ifstream ifs(Path + "/tag.json");
            if(ifs.good())
            {
                auto jtag = JSON::parse(ifs);
                std::stringstream strm;
                bool randomuuid = jtag.value("randomUuid", false);
                amiibo.Infos.Tag.uuid_length = 10;
                amiibo.Infos.Tag.tag_type = 2;
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
                        amiibo.Infos.Tag.uuid[i / 2] = tmpbyte & 0xFF;
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
                        amiibo.Infos.Model.amiibo_id[i / 2] = tmpbyte & 0xFF;
                        strm.str("");
                        strm.clear();
                    }
                    ifs.close();
                    ifs = std::ifstream(Path + "/register.json");
                    if(ifs.good())
                    {
                        auto jreg = JSON::parse(ifs);
                        std::string tmpname = jreg["name"];
                        strcpy(amiibo.Infos.Register.amiibo_name, tmpname.c_str());
                        std::string miifile = jreg["miiCharInfo"];
                        FILE *f = fopen((Path + "/" + miifile).c_str(), "rb");
                        if(f)
                        {
                            fread(&amiibo.Infos.Register.mii, 1, sizeof(NfpuMiiCharInfo), f);
                            fclose(f);
                        }
                        else
                        {
                            memcpy(&amiibo.Infos.Register.mii, DefaultCharInfo, sizeof(NfpuMiiCharInfo));

                            auto rc = mii::Initialize();
                            if(rc == 0)
                            {
                                mii::BuildRandom(&amiibo.Infos.Register.mii);
                                // Since mii service generates random miis with name "no name", change to "emuiibo"
                                const char *name = "emuiibo";
                                utf8_to_utf16(amiibo.Infos.Register.mii.mii_name, (const u8*)name, strlen(name));
                                mii::Finalize();
                            }

                            FILE *f = fopen((Path + "/" + miifile).c_str(), "wb");
                            if(f)
                            {
                                fwrite(&amiibo.Infos.Register.mii, 1, sizeof(NfpuMiiCharInfo), f);
                                fclose(f);
                            }
                        }
                        
                        std::string tmpdate = jreg["firstWriteDate"];
                        amiibo.Infos.Register.first_write_year = (u16)std::stoi(tmpdate.substr(0, 4));
                        amiibo.Infos.Register.first_write_month = (u8)std::stoi(tmpdate.substr(5, 2));
                        amiibo.Infos.Register.first_write_day = (u8)std::stoi(tmpdate.substr(8, 2));
                        ifs.close();
                        ifs = std::ifstream(Path + "/common.json");
                        if(ifs.good())
                        {
                            auto jcommon = JSON::parse(ifs);
                            tmpdate = jcommon["lastWriteDate"];
                            amiibo.Infos.Common.last_write_year = (u16)std::stoi(tmpdate.substr(0, 4));
                            amiibo.Infos.Common.last_write_month = (u8)std::stoi(tmpdate.substr(5, 2));
                            amiibo.Infos.Common.last_write_day = (u8)std::stoi(tmpdate.substr(8, 2));
                            amiibo.Infos.Common.write_counter = (u16)jcommon["writeCounter"];
                            amiibo.Infos.Common.version = (u16)jcommon["version"];
                            amiibo.Infos.Common.application_area_size = 0xD8;
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