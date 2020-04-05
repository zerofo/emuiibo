
#pragma once
#include <emu_Types.hpp>
#include <fs/fs_FileSystem.hpp>

namespace amiibo {

    using AreaId = u32;

    class AreaManager {

        public:
            static inline constexpr u32 DefaultSize = 0xD8;

        private:
            std::string dir;

            inline std::string EncodeAreaDirectory() {
                return fs::Concat(this->dir, "areas");
            }

            inline std::string EncodeAreaName(AreaId id) {
                std::stringstream strm;
                strm << "0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << id << ".bin";
                return strm.str();
            }

            inline std::string EncodeAreaFilePath(AreaId id) {
                return fs::Concat(this->EncodeAreaDirectory(), EncodeAreaName(id));
            }

            void CreateImpl(AreaId id, const void *data, size_t size, bool recreate);

        public:
            AreaManager() {}

            AreaManager(const std::string &amiibo_dir) : dir(amiibo_dir) {
                fs::CreateDirectory(this->EncodeAreaDirectory());
            }

            inline void Create(AreaId id, const void *data, size_t size) {
                this->CreateImpl(id, data, size, false);
            }
            
            inline void Recreate(AreaId id, const void *data, size_t size) {
                this->CreateImpl(id, data, size, true);
            }

            bool Exists(AreaId id);
            void Read(AreaId id, void *data, size_t size);
            void Write(AreaId id, const void *data, size_t size);
            size_t GetSize(AreaId id);

    };

}