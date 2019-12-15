#include "emu/emu_Status.hpp"
#include <stratosphere.hpp>

namespace emu
{
    static EmulationStatus Status = EmulationStatus::Off;
    static ams::os::Mutex StatusLock;

    // Both functions guarded by the mutex

    EmulationStatus GetStatus() LOCK_SCOPED(StatusLock,
    {
        return Status;
    })

    void SetStatus(EmulationStatus NewStatus) LOCK_SCOPED(StatusLock,
    {
        Status = NewStatus;
    })

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

    bool IsStatusOff()
    {
        return (GetStatus() == EmulationStatus::Off);
    }
}