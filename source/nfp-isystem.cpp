#include <atomic>
#include <cstdio>
#include <array>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <switch.h>
#include "nfp-isystem.hpp"

extern IEvent *g_eactivate;
extern u32 g_toggleEmulation;
extern HosMutex g_toggleLock;

NfpISystem::NfpISystem()
{
    cur_id = 0;
    edeactivate = CreateWriteOnlySystemEvent();
    eavailabilitychange = CreateWriteOnlySystemEvent();
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Unavailable;
}

NfpISystem::~NfpISystem()
{
    delete edeactivate;
    delete eavailabilitychange;
}

Result NfpISystem::Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf)
{
    state = NfpuState_Initialized;
    dvstate = NfpuDeviceState_Initialized;
    return 0;
}

Result NfpISystem::Finalize()
{
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Finalized;
    return 0;
}

Result NfpISystem::ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u32> out_count)
{
    u64 dvcid = 0x20;
    hidScanInput();
    if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) dvcid = (u64)CONTROLLER_PLAYER_1;
    memcpy(out_devices.pointer, &dvcid, sizeof(u64));
    out_count.SetValue(1);
    return 0;
}

Result NfpISystem::StartDetection(NfpDeviceHandle handle)
{
    g_eactivate->Signal();
    eavailabilitychange->Signal();
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpISystem::StopDetection(NfpDeviceHandle handle)
{
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

Result NfpISystem::Mount(NfpDeviceHandle handle, NfpuDeviceType type, NfpuMountTarget target)
{
    g_eactivate->Signal();
    dvstate = NfpuDeviceState_TagMounted;
    return 0;
}

Result NfpISystem::Unmount(NfpDeviceHandle handle)
{
    edeactivate->Signal();
    dvstate = NfpuDeviceState_SearchingForTag;
    return 0;
}

Result NfpISystem::Flush(NfpDeviceHandle handle)
{
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpISystem::Restore(NfpDeviceHandle handle)
{
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpISystem::GetTagInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuTagInfo, 0x1> out_info)
{
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuTagInfo tinfo = lyt.ProcessTagInfo();
    if(tinfo.uuid_length == 0) return LibnxError_NotFound;
    memcpy(out_info.pointer, &tinfo, sizeof(NfpuTagInfo));
    return 0;
}

Result NfpISystem::GetRegisterInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info)
{
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuRegisterInfo rinfo = lyt.ProcessRegisterInfo();
    memcpy(out_info.pointer, &rinfo, sizeof(NfpuRegisterInfo));
    return 0;
}

Result NfpISystem::GetModelInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuModelInfo, 0x1> out_info)
{
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuModelInfo minfo = lyt.ProcessModelInfo();
    memcpy(out_info.pointer, &minfo, sizeof(NfpuModelInfo));
    return 0;
}

Result NfpISystem::GetCommonInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuCommonInfo, 0x1> out_info)
{
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuCommonInfo cinfo = lyt.ProcessCommonInfo();
    memcpy(out_info.pointer, &cinfo, sizeof(NfpuCommonInfo));
    return 0;
}

Result NfpISystem::AttachActivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event)
{
    event.SetValue(g_eactivate->GetHandle());
    return 0;
}

Result NfpISystem::AttachDeactivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event)
{
    event.SetValue(edeactivate->GetHandle());
    return 0;
}

Result NfpISystem::GetState(Out<u32> out_state)
{
    out_state.SetValue(static_cast<u32>(state));
    return 0;
}

Result NfpISystem::GetDeviceState(NfpDeviceHandle handle, Out<u32> out_state)
{
    out_state.SetValue(static_cast<u32>(dvstate));
    return 0;
}

Result NfpISystem::GetNpadId(NfpDeviceHandle handle, Out<u32> out_npad_id)
{
    out_npad_id.SetValue((u32)handle.handle);
    return 0;
}

Result NfpISystem::AttachAvailabilityChangeEvent(Out<CopiedHandle> event)
{
    event.SetValue(eavailabilitychange->GetHandle());
    return 0;
}

Result NfpISystem::Format(NfpDeviceHandle handle)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> Format(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    ofs.close();
    return 0; // Format...??
}

Result NfpISystem::GetAdminInfo(NfpDeviceHandle handle, OutPointerWithServerSize<AdminInfo, 0x1> info)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> GetAdminInfo(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    ofs.close();
    memset(info.pointer, 0, sizeof(AdminInfo));
    std::vector<u8> data = { 0, 0xe0, 0x0e, 0, 0, 0, 0x04, 0, 0, 0x0e, 0x11, 0x10, 0x0c, 0, 0x03, 0x02 };
    memcpy(info.pointer, data.data(), 0x10);
    return 0;
}

Result NfpISystem::SetRegisterInfo(NfpDeviceHandle handle, InPointer<NfpuRegisterInfo> info)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> SetRegInfo(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    ofs.close();
    return 0; // DO THIS!
}

Result NfpISystem::DeleteRegisterInfo(NfpDeviceHandle handle)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> DeleteRegInfo(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    ofs.close();
    return 0; // Delete reginfo?
}

Result NfpISystem::DeleteApplicationArea(NfpDeviceHandle handle)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> DeleteAppArea(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    ofs.close();
    return 0; // DO THIS!
}

Result NfpISystem::ExistsApplicationArea(NfpDeviceHandle handle, Out<u8> exists)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> ExistsAppArea(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    ofs.close();

    exists.SetValue(1);
    return 0;
}