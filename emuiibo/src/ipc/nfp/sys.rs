use nx::result::*;
use nx::mem;
use nx::ipc::sf;
use nx::ipc::server;
use nx::ipc::sf::applet;
use nx::ipc::sf::nfp;
use nx::ipc::sf::nfp::ISystem;
use nx::ipc::sf::nfp::ISystemManager;
use nx::ipc::sf::sm;

use crate::emu;
use super::EmulationHandler;

pub struct System {
    handler: EmulationHandler
}

impl System {
    pub fn new(application_id: u64) -> Result<Self> {
        Ok(Self {
            handler: EmulationHandler::new(application_id)?
        })
    }
}

impl sf::IObject for System {
    fn get_session(&mut self) -> &mut sf::Session {
        self.handler.get_session()
    }

    fn get_command_table(&self) -> sf::CommandMetadataTable {
        vec! [
            ipc_cmif_interface_make_command_meta!(initialize_system: 0),
            ipc_cmif_interface_make_command_meta!(finalize_system: 1),
            ipc_cmif_interface_make_command_meta!(list_devices: 2),
            ipc_cmif_interface_make_command_meta!(start_detection: 3),
            ipc_cmif_interface_make_command_meta!(stop_detection: 4),
            ipc_cmif_interface_make_command_meta!(mount: 5),
            ipc_cmif_interface_make_command_meta!(unmount: 6),
            ipc_cmif_interface_make_command_meta!(flush: 10),
            ipc_cmif_interface_make_command_meta!(restore: 11),
            ipc_cmif_interface_make_command_meta!(get_tag_info: 13),
            ipc_cmif_interface_make_command_meta!(get_register_info: 14),
            ipc_cmif_interface_make_command_meta!(get_common_info: 15),
            ipc_cmif_interface_make_command_meta!(get_model_info: 16),
            ipc_cmif_interface_make_command_meta!(attach_activate_event: 17),
            ipc_cmif_interface_make_command_meta!(attach_deactivate_event: 18),
            ipc_cmif_interface_make_command_meta!(get_state: 19),
            ipc_cmif_interface_make_command_meta!(get_device_state: 20),
            ipc_cmif_interface_make_command_meta!(get_npad_id: 21),
            ipc_cmif_interface_make_command_meta!(attach_availability_change_event: 23, [(3, 0, 0) =>]),
            ipc_cmif_interface_make_command_meta!(format: 100),
            ipc_cmif_interface_make_command_meta!(get_admin_info: 101),
            ipc_cmif_interface_make_command_meta!(get_register_info_private: 102),
            ipc_cmif_interface_make_command_meta!(set_register_info_private: 103),
            ipc_cmif_interface_make_command_meta!(delete_register_info: 104),
            ipc_cmif_interface_make_command_meta!(delete_application_area: 105),
            ipc_cmif_interface_make_command_meta!(exists_application_area: 106),
        ]
    }
}

impl ISystem for System {
    fn initialize_system(&mut self, aruid: applet::AppletResourceUserId, process_id: sf::ProcessId, mcu_data: sf::InMapAliasBuffer) -> Result<()> {
        self.handler.initialize(aruid, process_id, mcu_data)
    }

    fn finalize_system(&mut self) -> Result<()> {
        self.handler.finalize()
    }

    fn list_devices(&mut self, out_devices: sf::OutPointerBuffer) -> Result<u32> {
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

    fn get_npad_id(&mut self, device_handle: nfp::DeviceHandle) -> Result<u32> {
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

pub struct SystemManager {
    session: sf::Session,
    info: sm::MitmProcessInfo
}

impl sf::IObject for SystemManager {
    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.session
    }

    fn get_command_table(&self) -> sf::CommandMetadataTable {
        vec! [
            ipc_cmif_interface_make_command_meta!(create_system_interface: 0)
        ]
    }
}

impl server::IMitmServerObject for SystemManager {
    fn new(info: sm::MitmProcessInfo) -> Self {
        Self { session: sf::Session::new(), info: info }
    }
}

impl ISystemManager for SystemManager {
    fn create_system_interface(&mut self) -> Result<mem::Shared<dyn sf::IObject>> {
        let system = System::new(self.info.program_id)?;
        Ok(mem::Shared::new(system))
    }
}

impl server::IMitmService for SystemManager {
    fn get_name() -> &'static str {
        nul!("nfp:sys")
    }

    fn should_mitm(_info: sm::MitmProcessInfo) -> bool {
        emu::get_emulation_status() == emu::EmulationStatus::On
    }
}