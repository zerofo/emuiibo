
#pragma once
#include <emu_Types.hpp>
#include <iomanip>
#include <sys/stat.h>

namespace fs {

    inline void CreateDirectory(const std::string &path) {
        mkdir(path.c_str(), 777);
    }

    template<mode_t Mode>
    inline bool StatImpl(const std::string &path) {
        struct stat st;
        if(stat(path.c_str(), &st) == 0) {
            if(st.st_mode & Mode) {
                return true;
            }
        }
        return false;
    }

    inline bool IsFile(const std::string &path) {
        return StatImpl<S_IFREG>(path);
    }

    inline bool IsDirectory(const std::string &path) {
        return StatImpl<S_IFDIR>(path);
    }

    inline bool MatchesExtension(const std::string &path, const std::string &ext) {
        return path.substr(path.find_last_of(".") + 1) == ext;
    }

    inline void DeleteFile(const std::string &path) {
        remove(path.c_str());
    }

    inline void DeleteDirectory(const std::string &path) {
        fsdevDeleteDirectoryRecursively(path.c_str());
    }

    inline void RecreateDirectory(const std::string &path) {
        DeleteDirectory(path);
        CreateDirectory(path);
    }

    inline void CreateEmptyFile(const std::string &path) {
        std::ofstream ofs(path);
    }

    inline size_t GetFileSize(const std::string &path) {
        struct stat st;
        if(stat(path.c_str(), &st) == 0) {
            return st.st_size;
        }
        return 0;
    }

    inline void ConcatImpl(std::string &base, const std::string &p) {
        if(base.back() != '/') {
            if(p.front() != '/') {
                base += '/';
            }
        }
        base += p;
    }

    template<typename ...Ps>
    inline std::string Concat(const std::string &base, Ps &&...paths) {
        // Generate a copy
        auto ret = base;
        (ConcatImpl(ret, paths), ...);
        return ret;
    }

    inline JSON LoadJSONFile(const std::string &path) {
        std::ifstream ifs(path);
        if(ifs.good()) {
            return JSON::parse(ifs);
        }
        return JSON::object();
    }

    inline void SaveJSONFile(const std::string &path, JSON &json) {
        std::ofstream ofs(path);
        if(ofs.good()) {
            ofs << std::setw(4) << json;
        }
    }

    template<typename T>
    inline void Save(const std::string &path, T t) {
        auto f = fopen(path.c_str(), "wb");
        if(f) {
            fwrite(&t, 1, sizeof(T), f);
            fclose(f);
        }
    }

    template<typename T>
    inline T Read(const std::string &path) {
        T t = T();
        const size_t file_sz = GetFileSize(path);
        if(file_sz >= sizeof(T)) {
            auto f = fopen(path.c_str(), "rb");
            if(f) {
                fread(&t, 1, sizeof(T), f);
                fclose(f);
            }
        }
        return t;
    }

    inline void EnsureEmuiiboDirectories() {
        CreateDirectory(consts::EmuDir);
        CreateDirectory(consts::AmiiboDir);
        CreateDirectory(consts::DumpedMiisDir);
    }

}