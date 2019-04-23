#include <atomic>
#include <cstdio>
#include <array>
#include <sys/stat.h>
#include <switch.h>
#include "nfp-iuser.hpp"

extern IEvent *g_eactivate;
extern u32 g_toggleEmulation;

HosMutex g_toggleLock;

NfpIUser::NfpIUser()
{
    cur_id = 0;
    edeactivate = CreateWriteOnlySystemEvent();
    eavailabilitychange = CreateWriteOnlySystemEvent();
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Unavailable;
    nfpCreateDebugInterface(&dbgsession);
}

NfpIUser::~NfpIUser()
{
    nfpDebugClose(&dbgsession);
    delete edeactivate;
    delete eavailabilitychange;
}

Result NfpIUser::Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugInitialize(&dbgsession, 0, buf.buffer, buf.num_elements);
    state = NfpuState_Initialized;
    dvstate = NfpuDeviceState_Initialized;
    return 0;
}

Result NfpIUser::Finalize()
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugFinalize(&dbgsession);
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Finalized;
    return 0;
}

Result NfpIUser::ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u32> out_count)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugListDevices(&dbgsession, out_count.GetPointer(), out_devices.pointer, out_devices.num_elements);
    u64 dvcid = 0x20;
    hidScanInput();
    if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) dvcid = (u64)CONTROLLER_PLAYER_1;
    memcpy(out_devices.pointer, &dvcid, sizeof(u64));
    out_count.SetValue(1);
    return 0;
}

Result NfpIUser::StartDetection(NfpDeviceHandle handle)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugStartDetection(&dbgsession, handle.handle);
    g_eactivate->Signal();
    eavailabilitychange->Signal();
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpIUser::StopDetection(NfpDeviceHandle handle)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugStopDetection(&dbgsession, handle.handle);
    switch(dvstate)
    {
        case NfpuDeviceState_TagFound:
        case NfpuDeviceState_TagMounted:
            edeactivate->Signal();
        case NfpuDeviceState_SearchingForTag:
        case NfpuDeviceState_TagRemoved:
            dvstate = NfpuDeviceState_Initialized;
            break;
        default:
            break;
    }
    return 0;
}

Result NfpIUser::Mount(NfpDeviceHandle handle, NfpuDeviceType type, NfpuMountTarget target)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugMount(&dbgsession, handle.handle, (u32)type, (u32)target);
    g_eactivate->Signal();
    dvstate = NfpuDeviceState_TagMounted;
    return 0;
}

Result NfpIUser::Unmount(NfpDeviceHandle handle)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugUnmount(&dbgsession, handle.handle);
    edeactivate->Signal();
    dvstate = NfpuDeviceState_SearchingForTag;
    return 0;
}

Result NfpIUser::OpenApplicationArea(Out<u32> npad_id, NfpDeviceHandle handle, u32 id)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugOpenApplicationArea(&dbgsession, handle.handle, id, npad_id.GetPointer());
    if(AmiiboEmulator::ExistsArea(id))
    {
        cur_id = id;
        npad_id.SetValue(handle.handle);
        return 0;
    }
    return NfpResults::ResultAreaNotFound;
}

Result NfpIUser::GetApplicationArea(OutBuffer<u8> data, Out<u32> data_size, NfpDeviceHandle handle)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugGetApplicationArea(&dbgsession, handle.handle, data_size.GetPointer(), data.buffer, data.num_elements);
    if(cur_id == 0) return NfpResults::ResultAreaNotFound;
    u64 sz = AmiiboEmulator::GetAreaSize(cur_id);
    if(sz == 0) return NfpResults::ResultAreaNotFound;
    AmiiboEmulator::ReadArea(cur_id, sz, data.buffer);
    data_size.SetValue(sz);
    return 0;
}

Result NfpIUser::SetApplicationArea(InBuffer<u8> data, NfpDeviceHandle handle)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugSetApplicationArea(&dbgsession, handle.handle, data.buffer, data.num_elements);
    if(cur_id == 0) return NfpResults::ResultAreaNotFound;
    AmiiboEmulator::WriteArea(cur_id, data.num_elements, data.buffer);
    return 0;
}

Result NfpIUser::Flush(NfpDeviceHandle handle)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugFlush(&dbgsession, handle.handle);
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpIUser::Restore(NfpDeviceHandle handle)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugRestore(&dbgsession, handle.handle);
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpIUser::CreateApplicationArea(InBuffer<u8> data, NfpDeviceHandle handle, u32 id)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugCreateApplicationArea(&dbgsession, handle.handle, id, data.buffer, data.num_elements);
    if(AmiiboEmulator::ExistsArea(id)) return NfpResults::ResultAreaAlreadyCreated;
    AmiiboEmulator::CreateArea(id, data.buffer, data.num_elements, false);
    return 0;
}

Result NfpIUser::GetTagInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuTagInfo, 0x1> out_info)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugGetTagInfo(&dbgsession, handle.handle, out_info.pointer);
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuTagInfo tinfo = lyt.ProcessTagInfo();
    if(tinfo.uuid_length == 0) return LibnxError_NotFound;
    memcpy(out_info.pointer, &tinfo, sizeof(NfpuTagInfo));
    return 0;
}

Result NfpIUser::GetRegisterInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugGetRegisterInfo(&dbgsession, handle.handle, out_info.pointer);
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuRegisterInfo rinfo = lyt.ProcessRegisterInfo();
    memcpy(out_info.pointer, &rinfo, sizeof(NfpuRegisterInfo));
    return 0;
}

Result NfpIUser::GetModelInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuModelInfo, 0x1> out_info)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugGetModelInfo(&dbgsession, handle.handle, out_info.pointer);
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuModelInfo minfo = lyt.ProcessModelInfo();
    memcpy(out_info.pointer, &minfo, sizeof(NfpuModelInfo));
    return 0;
}

Result NfpIUser::GetCommonInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuCommonInfo, 0x1> out_info)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugGetCommonInfo(&dbgsession, handle.handle, out_info.pointer);
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuCommonInfo cinfo = lyt.ProcessCommonInfo();
    memcpy(out_info.pointer, &cinfo, sizeof(NfpuCommonInfo));
    return 0;
}

Result NfpIUser::AttachActivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0)
    {
        Handle evh;
        Result rc = nfpDebugAttachActivateEvent(&dbgsession, handle.handle, &evh);
        event.SetValue(evh);
        return rc;
    }
    event.SetValue(g_eactivate->GetHandle());
    return 0;
}

Result NfpIUser::AttachDeactivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0)
    {
        Handle evh;
        Result rc = nfpDebugAttachDeactivateEvent(&dbgsession, handle.handle, &evh);
        event.SetValue(evh);
        return rc;
    }
    event.SetValue(edeactivate->GetHandle());
    return 0;
}

Result NfpIUser::GetState(Out<u32> out_state)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugGetState(&dbgsession, out_state.GetPointer());
    out_state.SetValue(static_cast<u32>(state));
    return 0;
}

Result NfpIUser::GetDeviceState(NfpDeviceHandle handle, Out<u32> out_state)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugGetDeviceState(&dbgsession, handle.handle, out_state.GetPointer());
    out_state.SetValue(static_cast<u32>(dvstate));
    return 0;
}

Result NfpIUser::GetNpadId(NfpDeviceHandle handle, Out<u32> out_npad_id)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugGetNpadId(&dbgsession, handle.handle, out_npad_id.GetPointer());
    out_npad_id.SetValue((u32)handle.handle);
    return 0;
}

Result NfpIUser::AttachAvailabilityChangeEvent(Out<CopiedHandle> event)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0)
    {
        Handle evh;
        Result rc = nfpDebugAttachAvailabilityChangeEvent(&dbgsession, &evh);
        event.SetValue(evh);
        return rc;
    }
    event.SetValue(eavailabilitychange->GetHandle());
    return 0;
}

Result NfpIUser::GetApplicationAreaSize(NfpDeviceHandle handle, Out<u32> size)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugGetApplicationAreaSize(&dbgsession, handle.handle, size.GetPointer());
    if(cur_id == 0) return NfpResults::ResultAreaNotFound;
    if(!AmiiboEmulator::ExistsArea(cur_id)) return NfpResults::ResultAreaNotFound;
    size.SetValue(AmiiboEmulator::GetAreaSize(cur_id));
    return 0;
}

Result NfpIUser::RecreateApplicationArea(InBuffer<u8> data, NfpDeviceHandle handle, u32 id)
{
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return nfpDebugRecreateApplicationArea(&dbgsession, handle.handle, id, data.buffer, data.num_elements);
    if(AmiiboEmulator::ExistsArea(id)) return NfpResults::ResultAreaAlreadyCreated;
    AmiiboEmulator::CreateArea(id, data.buffer, data.num_elements, true);
    return 0;
}