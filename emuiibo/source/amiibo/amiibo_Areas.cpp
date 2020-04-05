#include <amiibo/amiibo_Areas.hpp>

namespace amiibo {

    void AreaManager::CreateImpl(AreaId id, const void *data, size_t size, bool recreate) {
        auto area_path = this->EncodeAreaFilePath(id);
        if(recreate) {
            fs::DeleteFile(area_path.c_str());
        }
        this->Write(id, data, size);
    }

    bool AreaManager::Exists(AreaId id) {
        auto area_path = this->EncodeAreaFilePath(id);
        return fs::IsFile(area_path);
    }

    void AreaManager::Read(AreaId id, void *data, size_t size) {
        auto area_path = this->EncodeAreaFilePath(id);
        auto area_size = this->GetSize(id);
        auto read_sz = std::min(area_size, size);
        auto f = fopen(area_path.c_str(), "rb");
        if(f) {
            fread(data, 1, read_sz, f);
            fclose(f);
        }
    }

    void AreaManager::Write(AreaId id, const void *data, size_t size) {
        auto area_path = this->EncodeAreaFilePath(id);
        auto f = fopen(area_path.c_str(), "wb");
        if(f) {
            fwrite(data, 1, size, f);
            fclose(f);
        }
    }

    size_t AreaManager::GetSize(AreaId id) {
        auto area_path = this->EncodeAreaFilePath(id);
        return fs::GetFileSize(area_path);
    }

}