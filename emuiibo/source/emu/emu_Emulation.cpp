#include "emu/emu_Emulation.hpp"
#include "emu/emu_Consts.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace emu
{
    static int cur_amiibo_idx = -1;
    static Amiibo cur_amiibo = {};
    static int last_global_count = -1;
    static int last_appid_count = 0;
    static std::string custom_amiibo_path;
    static u64 current_appid = 0;
    static ams::os::Mutex emulation_lock;

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

    static std::string GetAmiiboPathForcur_amiibo_idx(u32 Idx, std::string Path)
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
        LOCK_SCOPED(emulation_lock,
        {
            mkdir(EmuDir.c_str(), 777);
            mkdir(AmiiboDir.c_str(), 777);
            mkdir(ConsoleMiisDir.c_str(), 777);
            DumpConsoleMiis();
            ProcessDirectory(EmuDir); // For <=0.2.x amiibos
            ProcessDirectory(AmiiboDir);
            last_global_count = GetAmiiboCount(AmiiboDir);
            if(current_appid > 0)
            {
                ProcessDirectory(AmiiboDir + "/" + MakeFromId(current_appid));
                last_appid_count = GetAmiiboCount(AmiiboDir + "/" + MakeFromId(current_appid));
            }
        })
        MoveToNextAmiibo();
    }

    Amiibo GetCurrentLoadedAmiibo() LOCK_SCOPED(emulation_lock,
    {
        return cur_amiibo;
    })

    bool MoveToNextAmiibo()
    {
        if(HasCustomAmiibo())
        {
            ResetCustomAmiibo();
            return MoveToNextAmiibo();
        }

        LOCK_SCOPED(emulation_lock,
        {
            auto dir = AmiiboDir;

            if((cur_amiibo_idx + 1) >= last_global_count)
            {
                if(last_appid_count > 0)
                {
                    if((cur_amiibo_idx + 1) >= (last_global_count + last_appid_count))
                    {
                        cur_amiibo_idx = 0;
                        dir = AmiiboDir;
                    }
                    else
                    {
                        cur_amiibo_idx++;
                        dir = AmiiboDir + "/" + MakeFromId(current_appid);
                    }
                }
                else
                {
                    cur_amiibo_idx = 0;
                    dir = AmiiboDir;
                }
            }
            else
            {
                cur_amiibo_idx++;
                dir = AmiiboDir;
            }

            int idx = cur_amiibo_idx;
            if(idx >= last_global_count) idx -= last_global_count;

            auto amiibo = GetAmiiboPathForcur_amiibo_idx(idx, dir);
            cur_amiibo = ProcessAmiibo(amiibo);
        })
        
        return true;
    }

    bool CanMoveNext()
    {
        if(HasCustomAmiibo()) return false;
        return ((cur_amiibo_idx + 1) < last_global_count);
    }

    void SetCustomAmiibo(std::string Path) LOCK_SCOPED(emulation_lock,
    {
        custom_amiibo_path = Path;
        cur_amiibo = ProcessAmiibo(custom_amiibo_path);
    })

    bool HasCustomAmiibo() LOCK_SCOPED(emulation_lock,
    {
        return !custom_amiibo_path.empty();
    })

    void ResetCustomAmiibo()
    {
        SetCustomAmiibo("");
    }

    void SetCurrentAppId(u64 Id)
    {
        bool has_custom = HasCustomAmiibo();
        LOCK_SCOPED(emulation_lock,
        {
            if(current_appid == Id) return;
            current_appid = Id;
            if(!has_custom) cur_amiibo_idx = -1;
        })
        if(!has_custom) Refresh();
    }

    u64 GetCurrentAppId() LOCK_SCOPED(emulation_lock,
    {
        return current_appid;
    })
}