 
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <emu/emu_Emulation.hpp>

namespace emu
{
    class IEmulationService final : public IServiceObject
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
            };

            Result GetCurrentAmiibo(OutBuffer<char> path, Out<bool> ok);
            Result SetCustomAmiibo(InBuffer<char> path);
            Result HasCustomAmiibo(Out<bool> has);
            Result ResetCustomAmiibo();
            Result SetEmulationOnForever();
            Result SetEmulationOnOnce();
            Result SetEmulationOff();
            Result MoveToNextAmiibo(Out<bool> ok);
            Result GetStatus(Out<u32> status);
            Result Refresh();

        public:
        
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(IEmulationService, GetCurrentAmiibo),
                MAKE_SERVICE_COMMAND_META(IEmulationService, SetCustomAmiibo),
                MAKE_SERVICE_COMMAND_META(IEmulationService, HasCustomAmiibo),
                MAKE_SERVICE_COMMAND_META(IEmulationService, ResetCustomAmiibo),
                MAKE_SERVICE_COMMAND_META(IEmulationService, SetEmulationOnForever),
                MAKE_SERVICE_COMMAND_META(IEmulationService, SetEmulationOnOnce),
                MAKE_SERVICE_COMMAND_META(IEmulationService, SetEmulationOff),
                MAKE_SERVICE_COMMAND_META(IEmulationService, MoveToNextAmiibo),
                MAKE_SERVICE_COMMAND_META(IEmulationService, GetStatus),
                MAKE_SERVICE_COMMAND_META(IEmulationService, Refresh),
            };
    };

    void CreateCommonActivateEvent();
}