#include <emu/emu_Status.hpp>
#include <stratosphere.hpp>

namespace emu
{
    static EmulationStatus Status = EmulationStatus::Off;
    static HosMutex StatusLock;

    EmulationStatus GetStatus()
    {
        std::scoped_lock<HosMutex> lock(StatusLock);
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

    bool IsStatusOff()
    {
        return (GetStatus() == EmulationStatus::Off);
    }
    
    void SetStatus(EmulationStatus NewStatus)
    {
        std::scoped_lock<HosMutex> lock(StatusLock);
        Status = NewStatus;
    }
}