#include <emu/emu_Emulation.hpp>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <emu/emu_Consts.hpp>

/*
template<typename ...FmtArgs>
static void LogLineFmt(std::string Fmt, FmtArgs &...Args)
{
    FILE *f = fopen("sdmc:/rw-emuiibo.log", "a");
    fprintf(f, (Fmt + "\n").c_str(), Args...);
    fclose(f);
}
*/

namespace emu
{
    static int Index = -1;
    static Amiibo CurrentAmiibo;
    static int LastCount = -1;
    static std::string CustomPath;

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

    static u32 GetAmiiboCount()
    {
        u32 count = 0;
        DIR *dp = opendir(AmiiboDir.c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                if(IsValidAmiibo(AmiiboDir + "/" + std::string(dt->d_name))) count++;
            }
            closedir(dp);
        }
        return count;
    }

    static std::string GetAmiiboPathForIndex(u32 Idx)
    {
        std::string out;
        u32 count = 0;
        DIR *dp = opendir(AmiiboDir.c_str());
        if(dp)
        {
            dirent *dt;
            while(true)
            {
                dt = readdir(dp);
                if(dt == NULL) break;
                auto item = AmiiboDir + "/" + std::string(dt->d_name);
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
        ProcessSingleDumps();
        LastCount = GetAmiiboCount();
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
        if(CanMoveNext())
        {
            Index++;
            auto amiibo = GetAmiiboPathForIndex(Index);
            CurrentAmiibo = ProcessAmiibo(amiibo);
            return true;
        }
        else if(((Index + 1) == LastCount) && (LastCount > 0))
        {
            Index = 0;
            auto amiibo = GetAmiiboPathForIndex(Index);
            CurrentAmiibo = ProcessAmiibo(amiibo);
            return true;
        }
        return false;
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
}