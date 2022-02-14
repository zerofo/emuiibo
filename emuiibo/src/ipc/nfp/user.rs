use nx::result::*;
use nx::mem;
use nx::ipc::sf;
use nx::ipc::server;
use nx::ipc::sf::applet;
use nx::ipc::sf::nfp;
use nx::ipc::sf::nfp::IUser;
use nx::ipc::sf::nfp::IUserManager;
use nx::ipc::sf::sm;

use crate::emu;
use super::EmulationHandler;

pub struct User {
    handler: EmulationHandler
}

impl User {
    pub fn new(application_id: u64) -> Result<Self> {
        emu::register_intercepted_application_id(application_id);
        
        Ok(Self {
            handler: EmulationHandler::new(application_id)?
        })
    }
}

impl Drop for User {
    fn drop(&mut self) {
        emu::unregister_intercepted_application_id(self.handler.get_application_id());
    }
}

impl sf::IObject for User {
    fn get_session(&mut self) -> &mut sf::Session {
        self.handler.get_session()
    }

    fn get_command_table(&self) -> sf::CommandMetadataTable {
        vec! [
            ipc_cmif_interface_make_command_meta!(initialize: 0),
            ipc_cmif_interface_make_command_meta!(finalize: 1),
            ipc_cmif_interface_make_command_meta!(list_devices: 2),
            ipc_cmif_interface_make_command_meta!(start_detection: 3),
            ipc_cmif_interface_make_command_meta!(stop_detection: 4),
            ipc_cmif_interface_make_command_meta!(mount: 5),
            ipc_cmif_interface_make_command_meta!(unmount: 6),
            ipc_cmif_interface_make_command_meta!(open_application_area: 7),
            ipc_cmif_interface_make_command_meta!(get_application_area: 8),
            ipc_cmif_interface_make_command_meta!(set_application_area: 9),
            ipc_cmif_interface_make_command_meta!(flush: 10),
            ipc_cmif_interface_make_command_meta!(restore: 11),
            ipc_cmif_interface_make_command_meta!(create_application_area: 12),
            ipc_cmif_interface_make_command_meta!(get_tag_info: 13),
            ipc_cmif_interface_make_command_meta!(get_register_info: 14),
            ipc_cmif_interface_make_command_meta!(get_common_info: 15),
            ipc_cmif_interface_make_command_meta!(get_model_info: 16),
            ipc_cmif_interface_make_command_meta!(attach_activate_event: 17),
            ipc_cmif_interface_make_command_meta!(attach_deactivate_event: 18),
            ipc_cmif_interface_make_command_meta!(get_state: 19),
            ipc_cmif_interface_make_command_meta!(get_device_state: 20),
            ipc_cmif_interface_make_command_meta!(get_npad_id: 21),
            ipc_cmif_interface_make_command_meta!(get_application_area_size: 22),
            ipc_cmif_interface_make_command_meta!(attach_availability_change_event: 23, [(3, 0, 0) =>]),
            ipc_cmif_interface_make_command_meta!(recreate_application_area: 24, [(3, 0, 0) =>])
        ]
    }
}

impl IUser for User {
    fn initialize(&mut self, aruid: applet::AppletResourceUserId, process_id: sf::ProcessId, mcu_data: sf::InMapAliasBuffer) -> Result<()> {
        self.handler.initialize(aruid, process_id, mcu_data)
    }

    fn finalize(&mut self) -> Result<()> {
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

    fn open_application_area(&mut self, device_handle: nfp::DeviceHandle, access_id: nfp::AccessId) -> Result<()> {
        self.handler.open_application_area(device_handle, access_id)
    }

    fn get_application_area(&mut self, device_handle: nfp::DeviceHandle, out_data: sf::OutMapAliasBuffer) -> Result<u32> {
        self.handler.get_application_area(device_handle, out_data)
    }

    fn set_application_area(&mut self, device_handle: nfp::DeviceHandle, data: sf::InMapAliasBuffer) -> Result<()> {
        self.handler.set_application_area(device_handle, data)
    }

    fn flush(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.flush(device_handle)
    }

    fn restore(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        self.handler.restore(device_handle)
    }

    fn create_application_area(&mut self, device_handle: nfp::DeviceHandle, access_id: nfp::AccessId, data: sf::InMapAliasBuffer) -> Result<()> {
        self.handler.create_application_area(device_handle, access_id, data)
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

    fn get_application_area_size(&mut self, device_handle: nfp::DeviceHandle) -> Result<u32> {
        self.handler.get_application_area_size(device_handle)
    }

    fn attach_availability_change_event(&mut self) -> Result<sf::CopyHandle> {
        self.handler.attach_availability_change_event()
    }

    fn recreate_application_area(&mut self, device_handle: nfp::DeviceHandle, access_id: nfp::AccessId, data: sf::InMapAliasBuffer) -> Result<()> {
        self.handler.recreate_application_area(device_handle, access_id, data)
    }
}

pub struct UserManager {
    session: sf::Session,
    info: sm::MitmProcessInfo
}

impl sf::IObject for UserManager {
    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.session
    }

    fn get_command_table(&self) -> sf::CommandMetadataTable {
        vec! [
            ipc_cmif_interface_make_command_meta!(create_user_interface: 0)
        ]
    }
}

impl server::IMitmServerObject for UserManager {
    fn new(info: sm::MitmProcessInfo) -> Self {
        Self { session: sf::Session::new(), info: info }
    }
}

impl IUserManager for UserManager {
    fn create_user_interface(&mut self) -> Result<mem::Shared<dyn sf::IObject>> {
        let user = User::new(self.info.program_id)?;
        Ok(mem::Shared::new(user))
    }
}

impl server::IMitmService for UserManager {
    fn get_name() -> &'static str {
        nul!("nfp:user")
    }

    fn should_mitm(_info: sm::MitmProcessInfo) -> bool {
        emu::get_emulation_status() == emu::EmulationStatus::On
    }
}