 
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include "emu/emu_Emulation.hpp"

namespace emu
{
    class IEmulationService final : public ams::sf::IServiceObject
    {
        private:

            enum class CommandId
            {
                GetCurrentAmiibo = 0,
                SetCustomAmiibo = 1,
                HasCustomAmiibo = 2,
                ResetCustomAmiibo = 3,
                SetEmulationOnForever = 4,
                SetEmulationOnOnce = 5,
                SetEmulationOff = 6,
                MoveToNextAmiibo = 7,
                GetStatus = 8,
                Refresh = 9,
                GetVersion = 10
            };

            ams::Result GetCurrentAmiibo(const ams::sf::OutBuffer &path);
            void SetCustomAmiibo(const ams::sf::InBuffer &path);
            void HasCustomAmiibo(ams::sf::Out<bool> out_has);
            void ResetCustomAmiibo();
            void SetEmulationOnForever();
            void SetEmulationOnOnce();
            void SetEmulationOff();
            ams::Result MoveToNextAmiibo();
            void GetStatus(ams::sf::Out<u32> out_status);
            void Refresh();
            void GetVersion(ams::sf::Out<Version> out_version);

        public:
        
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(GetCurrentAmiibo),
                MAKE_SERVICE_COMMAND_META(SetCustomAmiibo),
                MAKE_SERVICE_COMMAND_META(HasCustomAmiibo),
                MAKE_SERVICE_COMMAND_META(ResetCustomAmiibo),
                MAKE_SERVICE_COMMAND_META(SetEmulationOnForever),
                MAKE_SERVICE_COMMAND_META(SetEmulationOnOnce),
                MAKE_SERVICE_COMMAND_META(SetEmulationOff),
                MAKE_SERVICE_COMMAND_META(MoveToNextAmiibo),
                MAKE_SERVICE_COMMAND_META(GetStatus),
                MAKE_SERVICE_COMMAND_META(Refresh),
                MAKE_SERVICE_COMMAND_META(GetVersion)
            };
    };
}