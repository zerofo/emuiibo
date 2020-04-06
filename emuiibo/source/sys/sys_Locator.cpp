#include <sys/sys_Locator.hpp>
#include <fs/fs_FileSystem.hpp>
#include <amiibo/amiibo_Formats.hpp>
#include <dirent.h>
#include <vector>

namespace sys {

    static std::vector<std::string> g_cached_amiibos;
    static Lock g_cached_amiibos_lock;
    
    static std::vector<std::string> ListVirtualAmiibosImpl(const std::string &base_path) {
        EMU_LOCK_SCOPE_WITH(g_cached_amiibos_lock);
        std::vector<std::string> amiibos;
        auto dir = opendir(base_path.c_str());
        if(dir) {
            while(true) {
                auto dt = readdir(dir);
                if(dt == nullptr) {
                    break;
                }
                auto path = fs::Concat(base_path, dt->d_name);
                // Is this format valid?
                if(amiibo::VirtualAmiibo::IsValidVirtualAmiibo<amiibo::VirtualAmiibo>(path)) {
                    amiibos.push_back(path);
                }
                else {
                    // If it's a directory, scan and list amiibos there too
                    if(fs::IsDirectory(path)) {
                        auto inner_amiibos = ListVirtualAmiibosImpl(path);
                        amiibos.insert(amiibos.end(), inner_amiibos.begin(), inner_amiibos.end());
                    }
                }
            }
            closedir(dir);
        }
        return amiibos;
    }

    void UpdateVirtualAmiiboCache() {
        EMU_LOCK_SCOPE_WITH(g_cached_amiibos_lock);
        g_cached_amiibos = ListVirtualAmiibosImpl(consts::AmiiboDir);
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