#include <AmiiboUtil.hpp>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

namespace emuovl {

    inline bool DirectoryExists(const std::string &path) {
        struct stat st;
        return (stat(path.c_str(), &st) == 0) && (st.st_mode & S_IFDIR);
    }

    inline bool FileExists(const std::string &path) {
        struct stat st;
        return (stat(path.c_str(), &st) == 0) && (st.st_mode & S_IFREG);
    }

    VirtualAmiibo ProcessAmiibo(const std::string &path) {

        VirtualAmiibo amiibo = {};
        if(FileExists(path + "/model.json")) {
            if(FileExists(path + "/register.json")) {
                if(FileExists(path + "/common.json")) {
                    if(FileExists(path + "/tag.json")) {
                        std::ifstream ifs(path + "/register.json");
                        auto json = JSON::parse(ifs);
                        auto amiibo_name = json.value("name", "");
                        if(!amiibo_name.empty()) {
                            amiibo.path = path;
                            amiibo.amiibo_name = amiibo_name;
                        }
                    }
                }
            }
        }

        return amiibo;

    }

    std::vector<VirtualAmiibo> ListAmiibos() {
        std::vector<VirtualAmiibo> amiibos;

        auto dir = opendir("sdmc:/emuiibo/amiibo");
        if(dir) {
            dirent *dt;
            while(true) {
                dt = readdir(dir);
                if(dt == nullptr) {
                    break;
                }
                std::string path = "sdmc:/emuiibo/amiibo/";
                path += dt->d_name;
                auto amiibo = ProcessAmiibo(path);
                if(amiibo.IsValid()) {
                    amiibos.push_back(amiibo);
                }
            }
            closedir(dir);
        }

        return amiibos;

    }

}