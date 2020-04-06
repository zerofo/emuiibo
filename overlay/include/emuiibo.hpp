
#pragma once
#include <vector>
#include <string>
#include <switch.h>

namespace emu {

    struct VirtualAmiibo {
        Service srv;

        inline constexpr bool IsValid() {
            return serviceIsActive(&this->srv);
        }

        inline void SetAsActiveVirtualAmiibo() {
            serviceDispatch(&this->srv, 0);
        }

        inline std::string GetName() {
            char name[FS_MAX_PATH] = {0};
            serviceDispatch(&this->srv, 1,
                .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
                .buffers = { { name, FS_MAX_PATH } },
            );
            return name;
        }

        inline void Close() {
            serviceClose(&this->srv);
        }

    };

    enum class EmulationStatus : u32 {
        On,
        Off,
    };

    enum class VirtualAmiiboStatus : u32 {
        Invalid,
        Connected,
        Disconnected
    };

    struct Version {
        u8 major;
        u8 minor;
        u8 micro;
        bool dev_build;
    };

    bool IsAvailable();

    Result Initialize();
    void Exit();

    EmulationStatus GetEmulationStatus();
    void SetEmulationStatus(EmulationStatus status);

    Result GetActiveVirtualAmiibo(VirtualAmiibo &amiibo);
    void ResetActiveVirtualAmiibo();
    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus();
    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status);

    u32 GetVirtualAmiiboCount();
    Result OpenVirtualAmiibo(u32 idx, VirtualAmiibo &amiibo);

    Version GetVersion();

    // Utils

    inline std::vector<VirtualAmiibo> ListAmiibos() {
        std::vector<VirtualAmiibo> amiibos;
        auto count = GetVirtualAmiiboCount();
        for(u32 i = 0; i < count; i++) {
            VirtualAmiibo amiibo = {};
            if(R_FAILED(OpenVirtualAmiibo(i, amiibo))) {
                break;
            }
            amiibos.push_back(amiibo);
        }
        return amiibos;
    }

}