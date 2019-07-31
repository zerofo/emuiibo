#include <emu/emu_Emulation.hpp>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <emu/emu_Consts.hpp>
#include <sstream>

namespace emu
{
    static int Index = -1;
    static Amiibo CurrentAmiibo;
    static int LastCount = -1;
    static int LastIdCount = 0;
    static std::string CustomPath;
    static u64 CurrentId = 0;

    static std::string MakeFromId(u64 Id)
    {
        std::stringstream strm;
        strm << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << Id;
        return strm.str();
    }

    static bool ExistsFile(std::string Path)
    {
        struct stat st;
        return ((stat(Path.c_str(), &st) == 0) && (st.st_mode && S_IFREG));
    }

    static bool ExistsDir(std::string Path)
    {
        struct stat st;
        return ((stat(Path.c_str(), &st) == 0) && (st.st_mode && S_IFDIR));
    }

    static bool IsValidAmiibo(std::string Path)
    {
        return (ExistsDir(Path) && ExistsFile(Path + "/tag.json") && ExistsFile(Path + "/model.json") && ExistsFile(Path + "/register.json") && ExistsFile(Path + "/common.json"));
    }

    static u32 GetAmiiboCount(std::string Path)
    {
        u32 count = 0;
        DIR *dp = opendir(Path.c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                if(IsValidAmiibo(Path + "/" + std::string(dt->d_name))) count++;
            }
            closedir(dp);
        }
        return count;
    }

    static std::string GetAmiiboPathForIndex(u32 Idx, std::string Path)
    {
        std::string out;
        u32 count = 0;
        DIR *dp = opendir(Path.c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                auto item = Path + "/" + std::string(dt->d_name);
                if(IsValidAmiibo(item))
                {
                    if(Idx == count)
                    {
                        out = item;
                        break;
                    }
                    count++;
                }
            }
            closedir(dp);
        }
        return out;
    }

    void Refresh()
    {
        mkdir(EmuDir.c_str(), 777);
        mkdir(AmiiboDir.c_str(), 777);
        mkdir(ConsoleMiisDir.c_str(), 777);
        DumpConsoleMiis();
        ProcessDirectory(EmuDir); // For <=0.2.x amiibos
        ProcessDirectory(AmiiboDir);
        LastCount = GetAmiiboCount(AmiiboDir);
        if(CurrentId > 0)
        {
            ProcessDirectory(AmiiboDir + "/" + MakeFromId(CurrentId));
            LastIdCount = GetAmiiboCount(AmiiboDir + "/" + MakeFromId(CurrentId));
        }
        MoveToNextAmiibo();
    }

    Amiibo &GetCurrentLoadedAmiibo()
    {
        return CurrentAmiibo;
    }

    bool MoveToNextAmiibo()
    {
        if(HasCustomAmiibo())
        {
            ResetCustomAmiibo();
            return MoveToNextAmiibo();
        }

        auto dir = AmiiboDir;

        if((Index + 1) >= LastCount)
        {
            if(LastIdCount > 0)
            {
                if((Index + 1) >= (LastCount + LastIdCount))
                {
                    Index = 0;
                    dir = AmiiboDir;
                }
                else
                {
                    Index++;
                    dir = AmiiboDir + "/" + MakeFromId(CurrentId);
                }
            }
            else
            {
                Index = 0;
                dir = AmiiboDir;
            }
        }
        else
        {
            Index++;
            dir = AmiiboDir;
        }

        int idx = Index;
        if(idx >= LastCount) idx -= LastCount;

        auto amiibo = GetAmiiboPathForIndex(idx, dir);
        CurrentAmiibo = ProcessAmiibo(amiibo);
        
        return true;
    }

    bool CanMoveNext()
    {
        if(HasCustomAmiibo()) return false;
        return ((Index + 1) < LastCount);
    }

    void SetCustomAmiibo(std::string Path)
    {
        CustomPath = Path;
        CurrentAmiibo = ProcessAmiibo(CustomPath);
    }

    bool HasCustomAmiibo()
    {
        return !CustomPath.empty();
    }

    void ResetCustomAmiibo()
    {
        SetCustomAmiibo("");
    }

    void SetCurrentAppId(u64 Id)
    {
        if(CurrentId != Id)
        {
            CurrentId = Id;
            Index = -1;
            Refresh();
        }
    }

    u64 GetCurrentAppId()
    {
        return CurrentId;
    }
}