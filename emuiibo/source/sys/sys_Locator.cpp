#include <sys/sys_Locator.hpp>
#include <fs/fs_FileSystem.hpp>
#include <amiibo/amiibo_Formats.hpp>
#include <dirent.h>
#include <vector>

namespace sys {

    static std::vector<std::string> g_cached_amiibos;
    static Lock g_cached_amiibos_lock;

    static inline std::vector<std::string> ListVirtualAmiibosImpl() {
        EMU_LOCK_SCOPE_WITH(g_cached_amiibos_lock);
        std::vector<std::string> amiibos;
        auto dir = opendir(consts::AmiiboDir.c_str());
        if(dir) {
            while(true) {
                auto dt = readdir(dir);
                if(dt == nullptr) {
                    break;
                }
                auto path = fs::Concat(consts::AmiiboDir, dt->d_name);
                // Is this format valid?
                if(amiibo::VirtualAmiibo::IsValidVirtualAmiibo<amiibo::VirtualAmiibo>(path)) {
                    amiibos.push_back(path);
                }
            }
            closedir(dir);
        }
        return amiibos;
    }

    static inline void DoCacheAmiibos() {
        EMU_LOCK_SCOPE_WITH(g_cached_amiibos_lock);
        g_cached_amiibos = ListVirtualAmiibosImpl();
    }

    void UpdateVirtualAmiiboCache() {
        EMU_LOCK_SCOPE_WITH(g_cached_amiibos_lock);
        DoCacheAmiibos();
    }

    u32 GetVirtualAmiiboCount() {
        EMU_LOCK_SCOPE_WITH(g_cached_amiibos_lock);
        return g_cached_amiibos.size();
    }

    std::string GetVirtualAmiibo(u32 idx) {
        EMU_LOCK_SCOPE_WITH(g_cached_amiibos_lock);
        if(idx < g_cached_amiibos.size()) {
            return g_cached_amiibos[idx];
        }
        return "";
    }

}