#include "nfp/user/user_IUser.hpp"
#include "emu/emu_Emulation.hpp"
#include <atomic>
#include <cstdio>
#include <sys/stat.h>

namespace nfp::user
{
    ams::Result IUser::OpenApplicationArea(ams::sf::Out<u32> npad_id, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        LOG_FMT("Open area - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", exists area? " << std::boolalpha << amiibo.ExistsArea(id))
        if(amiibo.ExistsArea(id))
        {
            this->currentAreaAppId = id;
            npad_id.SetValue(handle.npad_id);
            return ams::ResultSuccess();
        }
        return result::ResultAreaNotFound;
    }

    ams::Result IUser::GetApplicationArea(const ams::sf::OutBuffer &data, ams::sf::Out<u32> data_size, DeviceHandle handle)
    {
        LOG_FMT("Get area - opened area: " << this->currentAreaAppId)
        if(this->currentAreaAppId == 0) return result::ResultAreaNotFound;
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        u64 sz = (u64)amiibo.GetAreaSize(this->currentAreaAppId);
        if(sz == 0) return result::ResultAreaNotFound;
        amiibo.ReadArea(this->currentAreaAppId, data.GetPointer(), sz);
        data_size.SetValue(sz);
        return ams::ResultSuccess();
    }

    ams::Result IUser::SetApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle)
    {
        LOG_FMT("Set area - opened area: " << this->currentAreaAppId)
        if(this->currentAreaAppId == 0) return result::ResultAreaNotFound;
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        amiibo.WriteArea(this->currentAreaAppId, (u8*)data.GetPointer(), data.GetSize());
        return ams::ResultSuccess();
    }

    ams::Result IUser::CreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        LOG_FMT("Create area - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", exists area? " << std::boolalpha << amiibo.ExistsArea(id))
        if(amiibo.ExistsArea(id)) return result::ResultAreaAlreadyCreated;
        amiibo.CreateArea(id, (u8*)data.GetPointer(), data.GetSize(), false);
        return ams::ResultSuccess();
    }

    ams::Result IUser::GetApplicationAreaSize(DeviceHandle handle, ams::sf::Out<u32> size)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        LOG_FMT("Get area - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", exists area? " << std::boolalpha << amiibo.ExistsArea(this->currentAreaAppId) << ", opened area: " << this->currentAreaAppId)
        if(this->currentAreaAppId == 0) return result::ResultAreaNotFound;
        if(!amiibo.ExistsArea(this->currentAreaAppId)) return result::ResultAreaNotFound;
        size.SetValue(amiibo.GetAreaSize(this->currentAreaAppId));
        return ams::ResultSuccess();
    }

    ams::Result IUser::RecreateApplicationArea(const ams::sf::InBuffer &data, DeviceHandle handle, u32 id)
    {
        auto amiibo = emu::GetCurrentLoadedAmiibo();
        LOG_FMT("Recreate area - is amiibo valid? " << std::boolalpha << amiibo.IsValid() << ", already exists area? " << std::boolalpha << amiibo.ExistsArea(id))
        amiibo.CreateArea(id, (u8*)data.GetPointer(), data.GetSize(), true);
        return ams::ResultSuccess();
    }
}