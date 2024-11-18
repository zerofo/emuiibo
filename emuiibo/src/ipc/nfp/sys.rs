use nx::ipc::sf::hid;
use nx::ipc::sf::ncm;
use nx::result::*;
use nx::mem;
use nx::ipc::sf;
use nx::ipc::server;
use nx::ipc::sf::applet;
use nx::ipc::sf::nfp;
use nx::ipc::sf::nfp::ISystemServer;
use nx::ipc::sf::nfp::ISystemManagerServer;
use nx::ipc::sf::sm;
use nx::service::nfp::System;

use crate::emu;
use super::EmulationHandler;

pub struct SystemEmulator {
    handler: EmulationHandler,
    dummy_session: sf::Session
}

impl SystemEmulator {
    pub fn new(application_id: ncm::ProgramId) -> Result<Self> {
        Ok(Self {
            handler: EmulationHandler::new(application_id)?,
            dummy_session: sf::Session::new()
        })
    }
}

impl sf::IObject for SystemEmulator {
    //ipc_sf_object_impl_default_command_metadata!();

    fn get_session(&self) -> &sf::Session {
        &self.dummy_session
    }

    fn get_session_mut(&mut self) -> &mut sf::Session {
        &mut self.dummy_session
    }
}

impl ISystemServer for SystemEmulator {
    fn initialize_system(&mut self, process_id: sf::AppletResourceUserId, _reserved: u64, mcu_data: sf::InMapAliasBuffer<nfp::McuVersionData>) -> Result<()> {
        self.handler.initialize(process_id, mcu_data)
    }

    fn finalize_system(&mut self) -> Result<()> {
        self.handler.finalize()
    }

    fn list_devices(&mut self, out_devices: sf::OutPointerBuffer<nfp::DeviceHandle>) -> Result<u32> {
        self.handler.list_devices(out_devices)
    }

    fn start_detection(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.start_detection(device_handle)
    }

    fn stop_detection(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.stop_detection(device_handle)
    }

    fn mount(&mut self, device_handle: nfp::DeviceHandle, model_type: nfp::ModelType, mount_target: nfp::MountTarget) -> Result<()> {
        self.handler.mount(device_handle, model_type, mount_target)
    }

    fn unmount(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.unmount(device_handle)
    }

    fn flush(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.flush(device_handle)
    }

    fn restore(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.restore(device_handle)
    }

    fn get_tag_info(&mut self, device_handle: nfp::DeviceHandle, out_tag_info: sf::OutFixedPointerBuffer<nfp::TagInfo>) -> Result<()> {
        self.handler.get_tag_info(device_handle, out_tag_info)
    }

    fn get_register_info(&mut self, device_handle: nfp::DeviceHandle, out_register_info: sf::OutFixedPointerBuffer<nfp::RegisterInfo>) -> Result<()> {
        self.handler.get_register_info(device_handle, out_register_info)
    }

    fn get_common_info(&mut self, device_handle: nfp::DeviceHandle, out_common_info: sf::OutFixedPointerBuffer<nfp::CommonInfo>) -> Result<()> {
        self.handler.get_common_info(device_handle, out_common_info)
    }

    fn get_model_info(&mut self, device_handle: nfp::DeviceHandle, out_model_info: sf::OutFixedPointerBuffer<nfp::ModelInfo>) -> Result<()> {
        self.handler.get_model_info(device_handle, out_model_info)
    }

    fn attach_activate_event(&mut self, device_handle: nfp::DeviceHandle) -> Result<sf::CopyHandle> {
        self.handler.attach_activate_event(device_handle)
    }

    fn attach_deactivate_event(&mut self, device_handle: nfp::DeviceHandle) -> Result<sf::CopyHandle> {
        self.handler.attach_deactivate_event(device_handle)
    }

    fn get_state(&mut self) -> Result<nfp::State> {
        self.handler.get_state()
    }

    fn get_device_state(&mut self, device_handle: nfp::DeviceHandle) -> Result<nfp::DeviceState> {
        self.handler.get_device_state(device_handle)
    }

    fn get_npad_id(&mut self, device_handle: nfp::DeviceHandle) -> Result<hid::NpadIdType> {
        self.handler.get_npad_id(device_handle)
    }

    fn attach_availability_change_event(&mut self) -> Result<sf::CopyHandle> {
        self.handler.attach_availability_change_event()
    }

    fn format(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.format(device_handle)
    }

    fn get_admin_info(&mut self, device_handle: nfp::DeviceHandle, out_admin_info: sf::OutFixedPointerBuffer<nfp::AdminInfo>) -> Result<()> {
        self.handler.get_admin_info(device_handle, out_admin_info)
    }

    fn get_register_info_private(&mut self, device_handle: nfp::DeviceHandle, out_register_info_private: sf::OutFixedPointerBuffer<nfp::RegisterInfoPrivate>) -> Result<()> {
        self.handler.get_register_info_private(device_handle, out_register_info_private)
    }

    fn set_register_info_private(&mut self, device_handle: nfp::DeviceHandle, register_info_private: sf::InFixedPointerBuffer<nfp::RegisterInfoPrivate>) -> Result<()> {
        self.handler.set_register_info_private(device_handle, register_info_private)
    }

    fn delete_register_info(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.delete_register_info(device_handle)
    }

    fn delete_application_area(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.delete_application_area(device_handle)
    }

    fn exists_application_area(&mut self, device_handle: nfp::DeviceHandle) -> Result<bool> {
        self.handler.exists_application_area(device_handle)
    }
}

impl server::ISessionObject for SystemEmulator {
    fn try_handle_request_by_id(&mut self, req_id: u32, protocol: nx::ipc::CommandProtocol, server_ctx: &mut server::ServerContext) -> Option<Result<()>> {
        <Self as ISystemServer>::try_handle_request_by_id(self, req_id, protocol, server_ctx)
    }
}

pub struct SystemManager {
    info: sm::mitm::MitmProcessInfo,
    dummy_session: sf::Session
}

impl sf::IObject for SystemManager {
    //ipc_sf_object_impl_default_command_metadata!();

    fn get_session(&self) -> &sf::Session {
        &self.dummy_session
    }

    fn get_session_mut(&mut self) -> &mut sf::Session {
        &mut self.dummy_session
    }
}

impl server::ISessionObject for SystemManager {
    fn try_handle_request_by_id(&mut self, req_id: u32, protocol: nx::ipc::CommandProtocol, server_ctx: &mut server::ServerContext) -> Option<Result<()>> {
        <Self as ISystemManagerServer>::try_handle_request_by_id(self, req_id, protocol, server_ctx)
    }
}

impl server::IMitmServerObject for SystemManager {
    fn new(info: sm::mitm::MitmProcessInfo) -> Self {
        Self {
            info,
            dummy_session: sf::Session::new()
        }
    }
}

use crate::nx::ipc::client::IClientObject;
impl ISystemManagerServer for SystemManager {
    fn create_system_interface(&mut self) -> Result<System> {
        Ok(System::new(sf::Session::new()))
    }
}

impl server::IMitmService for SystemManager {
    fn get_name() -> sm::ServiceName {
        sm::ServiceName::new("nfp:sys")
    }

    #[inline(never)]
    fn should_mitm(_info: sm::mitm::MitmProcessInfo) -> bool {
        emu::get_emulation_status() == emu::EmulationStatus::On
    }
}