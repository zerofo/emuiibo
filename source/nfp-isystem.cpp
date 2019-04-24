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
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> .ctor()" << std::endl;
    std::cout.rdbuf(rcout);
    cur_id = 0;
    edeactivate = CreateWriteOnlySystemEvent();
    eavailabilitychange = CreateWriteOnlySystemEvent();
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Unavailable;
}

NfpISystem::~NfpISystem()
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> .dtor()" << std::endl;
    std::cout.rdbuf(rcout);
    delete edeactivate;
    delete eavailabilitychange;
}

Result NfpISystem::Initialize(u64 aruid, u64 unk, PidDescriptor pid_desc, InBuffer<u8> buf)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> Initialize(aruid: " << aruid << ")" << std::endl;
    std::cout.rdbuf(rcout);
    state = NfpuState_Initialized;
    dvstate = NfpuDeviceState_Initialized;
    return 0;
}

Result NfpISystem::Finalize()
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> Finalize()" << std::endl;
    std::cout.rdbuf(rcout);
    state = NfpuState_NonInitialized;
    dvstate = NfpuDeviceState_Finalized;
    return 0;
}

Result NfpISystem::ListDevices(OutPointerWithClientSize<u64> out_devices, Out<u32> out_count)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> ListDevices()" << std::endl;
    std::cout.rdbuf(rcout);
    u64 dvcid = 0x20;
    hidScanInput();
    if(hidIsControllerConnected(CONTROLLER_PLAYER_1)) dvcid = (u64)CONTROLLER_PLAYER_1;
    memcpy(out_devices.pointer, &dvcid, sizeof(u64));
    out_count.SetValue(1);
    return 0;
}

Result NfpISystem::StartDetection(NfpDeviceHandle handle)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> StartDetection(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    g_eactivate->Signal();
    eavailabilitychange->Signal();
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpISystem::StopDetection(NfpDeviceHandle handle)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> StopDetection(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
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
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> Mount(handle: " << handle.handle << ", type: " << type << ", target: " << target << ")" << std::endl;
    std::cout.rdbuf(rcout);
    g_eactivate->Signal();
    eavailabilitychange->Signal();
    dvstate = NfpuDeviceState_TagMounted;
    return 0;
}

Result NfpISystem::Unmount(NfpDeviceHandle handle)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> Unmount(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    edeactivate->Signal();
    dvstate = NfpuDeviceState_SearchingForTag;
    return 0;
}

Result NfpISystem::Flush(NfpDeviceHandle handle)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> Flush(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    dvstate = NfpuDeviceState_SearchingForTag;
    return 0;
}

Result NfpISystem::Restore(NfpDeviceHandle handle)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> Restore(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    dvstate = NfpuDeviceState_TagFound;
    return 0;
}

Result NfpISystem::GetTagInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuTagInfo, 0x1> out_info)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> GetTagInfo(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuTagInfo tinfo = lyt.ProcessTagInfo();
    if(tinfo.uuid_length == 0) return LibnxError_NotFound;
    memcpy(out_info.pointer, &tinfo, sizeof(NfpuTagInfo));
    return 0;
}

Result NfpISystem::GetRegisterInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> GetRegisterInfo(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuRegisterInfo rinfo = lyt.ProcessRegisterInfo();
    memcpy(out_info.pointer, &rinfo, sizeof(NfpuRegisterInfo));
    return 0;
}

Result NfpISystem::GetModelInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuModelInfo, 0x1> out_info)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> GetModelInfo(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuModelInfo minfo = lyt.ProcessModelInfo();
    memcpy(out_info.pointer, &minfo, sizeof(NfpuModelInfo));
    return 0;
}

Result NfpISystem::GetCommonInfo(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuCommonInfo, 0x1> out_info)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> GetCommonInfo(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuCommonInfo cinfo = lyt.ProcessCommonInfo();
    memcpy(out_info.pointer, &cinfo, sizeof(NfpuCommonInfo));
    return 0;
}

Result NfpISystem::AttachActivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> AttachActivateEvent(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    event.SetValue(g_eactivate->GetHandle());
    return 0;
}

Result NfpISystem::AttachDeactivateEvent(NfpDeviceHandle handle, Out<CopiedHandle> event)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> AttachDeactivateEvent(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    event.SetValue(edeactivate->GetHandle());
    return 0;
}

Result NfpISystem::GetState(Out<u32> out_state)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> GetState()" << std::endl;
    std::cout.rdbuf(rcout);
    out_state.SetValue(static_cast<u32>(state));
    return 0;
}

Result NfpISystem::GetDeviceState(NfpDeviceHandle handle, Out<u32> out_state)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> GetDeviceState(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    out_state.SetValue(static_cast<u32>(dvstate));
    return 0;
}

Result NfpISystem::GetNpadId(NfpDeviceHandle handle, Out<u32> out_npad_id)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> GetNpadId(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
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
    std::vector<u8> data = { 0x34, 0xf8, 0x02, 0 };
    memcpy(info.pointer, data.data(), 4);
    return 0;
}

Result NfpISystem::GetRegisterInfo2(NfpDeviceHandle handle, OutPointerWithServerSize<NfpuRegisterInfo, 0x1> out_info)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    std::cout << "[nfp:sys] -> GetRegisterInfo2(handle: " << handle.handle << ")" << std::endl;
    std::cout.rdbuf(rcout);
    AmiiboLayout lyt = AmiiboEmulator::GetCurrentAmiibo();
    if(lyt.path.empty()) return NfpResults::ResultDeviceNotFound;
    NfpuRegisterInfo rinfo = lyt.ProcessRegisterInfo();
    memcpy(out_info.pointer, &rinfo, sizeof(NfpuRegisterInfo));
    return 0;
}

Result NfpISystem::SetRegisterInfo(NfpDeviceHandle handle, InPointer<NfpuRegisterInfo> info)
{
    std::ofstream ofs("sdmc:/emuiibo.log", std::fstream::in | std::fstream::out | std::fstream::app);
    auto rcout = std::cout.rdbuf();
    std::cout.rdbuf(ofs.rdbuf());
    NfpuRegisterInfo rinfo;
    memcpy(&rinfo, info.pointer, sizeof(NfpuRegisterInfo));
    std::cout << "[nfp:sys] -> SetRegInfo(handle: " << handle.handle << ", name: " << std::string(rinfo.amiibo_name) << ")" << std::endl;
    std::cout.rdbuf(rcout);
    ofs.close();
    return 0; // DO THIS!
}

Result NfpISystem::DeleteRegisterInfo(NfpDeviceHandle handle)
{
    return 0; // Delete reginfo?
}

Result NfpISystem::DeleteApplicationArea(NfpDeviceHandle handle)
{
    return 0; // DO THIS!
}

Result NfpISystem::ExistsApplicationArea(NfpDeviceHandle handle, Out<u8> exists)
{
    exists.SetValue(1);
    return 0;
}