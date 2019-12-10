#include "emu/emu_Status.hpp"
#include <stratosphere.hpp>

namespace emu
{
    static EmulationStatus Status = EmulationStatus::Off;
    static ams::os::Mutex StatusLock;

    EmulationStatus GetStatus()
    {
        LOCK(StatusLock)
        return Status;
    }

    bool IsStatusOnForever()
    {
        return (GetStatus() == EmulationStatus::OnForever);
    }

    bool IsStatusOnOnce()
    {
        return (GetStatus() == EmulationStatus::OnOnce);
    }

    bool IsStatusOn()
    {
        return (IsStatusOnForever() || IsStatusOnOnce());
    }

    bool IsStatusOff() LOCK_SCOPED(StatusLock,
    {
        return (GetStatus() == EmulationStatus::Off);
    })
    
    void SetStatus(EmulationStatus NewStatus) LOCK_SCOPED(StatusLock,
    {
        Status = NewStatus;
    })
}