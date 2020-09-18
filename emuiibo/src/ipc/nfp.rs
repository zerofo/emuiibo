use nx::result::*;
use nx::results;
use nx::mem;
use nx::ipc::sf;
use nx::ipc::server;
use nx::ipc::sf::applet;
use nx::ipc::sf::nfp;
use nx::ipc::sf::nfp::IUser;
use nx::ipc::sf::nfp::IUserManager;
use nx::ipc::sf::sm;
use nx::diag::log;
use nx::wait;
use nx::sync;
use nx::thread;

use crate::area;
use crate::emu;

pub struct User {
    session: sf::Session,
    application_id: u64,
    activate_event: wait::SystemEvent,
    deactivate_event: wait::SystemEvent,
    availability_change_event: wait::SystemEvent,
    state: sync::Locked<nfp::State>,
    device_state: sync::Locked<nfp::DeviceState>,
    should_end_thread: sync::Locked<bool>,
    current_opened_area: area::ApplicationArea,
    emu_handler_thread: thread::Thread
}

impl User {
    pub fn new(application_id: u64) -> Self {
        // diag_log!(log::LmLogger { log::LogSeverity::Trace, true } => "Registered intercepted: 0x{:016X}\n", application_id);
        emu::register_intercepted_application_id(application_id);
        Self { session: sf::Session::new(), application_id: application_id, activate_event: wait::SystemEvent::empty(), deactivate_event: wait::SystemEvent::empty(), availability_change_event: wait::SystemEvent::empty(), state: sync::Locked::new(false, nfp::State::NonInitialized), device_state: sync::Locked::new(false, nfp::DeviceState::Unavailable), should_end_thread: sync::Locked::new(false, false), emu_handler_thread: thread::Thread::empty(), current_opened_area: area::ApplicationArea::new() }
    }

    pub fn is_state(&mut self, state: nfp::State) -> bool {
        self.state.get_val() == state
    }

    pub fn is_device_state(&mut self, device_state: nfp::DeviceState) -> bool {
        self.device_state.get_val() == device_state
    }

    pub fn handle_virtual_amiibo_status(&mut self, status: emu::VirtualAmiiboStatus) {
        match status {
            emu::VirtualAmiiboStatus::Connected => match self.device_state.get_val() {
                nfp::DeviceState::SearchingForTag => {
                    self.device_state.set(nfp::DeviceState::TagFound);
                    self.activate_event.signal().unwrap();
                },
                _ => {}
            },
            emu::VirtualAmiiboStatus::Disconnected => match self.device_state.get_val() {
                nfp::DeviceState::TagFound | nfp::DeviceState::TagMounted => {
                    self.device_state.set(nfp::DeviceState::SearchingForTag);
                    self.deactivate_event.signal().unwrap();
                },
                _ => {}
            },
            _ => {}
        };
    }

    fn emu_handler_impl(user_v: *mut u8) {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Starting emu_handler thread...\n");
        let user = user_v as *mut User;
        unsafe {
            loop {
                if (*user).should_end_thread.get_val() {
                    break;
                }
                
                let status = emu::get_active_virtual_amiibo_status();
                (*user).handle_virtual_amiibo_status(status);
                let _ = thread::sleep(100_000_000);
            }
        }
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Exiting emu_handler thread...\n");
    }
}

impl Drop for User {
    fn drop(&mut self) {
        emu::unregister_intercepted_application_id(self.application_id);
        // diag_log!(log::LmLogger { log::LogSeverity::Trace, true } => "Unregistered intercepted: 0x{:016X}\n", self.application_id);
        self.should_end_thread.set(true);
        self.emu_handler_thread.join().unwrap();
    }
}

impl sf::IObject for User {
    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.session
    }

    fn get_command_table(&self) -> sf::CommandMetadataTable {
        vec! [
            ipc_interface_make_command_meta!(initialize: 0),
            ipc_interface_make_command_meta!(finalize: 1),
            ipc_interface_make_command_meta!(list_devices: 2),
            ipc_interface_make_command_meta!(start_detection: 3),
            ipc_interface_make_command_meta!(stop_detection: 4),
            ipc_interface_make_command_meta!(mount: 5),
            ipc_interface_make_command_meta!(unmount: 6),
            ipc_interface_make_command_meta!(open_application_area: 7),
            ipc_interface_make_command_meta!(get_application_area: 8),
            ipc_interface_make_command_meta!(set_application_area: 9),
            ipc_interface_make_command_meta!(flush: 10),
            ipc_interface_make_command_meta!(restore: 11),
            ipc_interface_make_command_meta!(create_application_area: 12),
            ipc_interface_make_command_meta!(get_tag_info: 13),
            ipc_interface_make_command_meta!(get_register_info: 14),
            ipc_interface_make_command_meta!(get_common_info: 15),
            ipc_interface_make_command_meta!(get_model_info: 16),
            ipc_interface_make_command_meta!(attach_activate_event: 17),
            ipc_interface_make_command_meta!(attach_deactivate_event: 18),
            ipc_interface_make_command_meta!(get_state: 19),
            ipc_interface_make_command_meta!(get_device_state: 20),
            ipc_interface_make_command_meta!(get_npad_id: 21),
            ipc_interface_make_command_meta!(get_application_area_size: 22),
            ipc_interface_make_command_meta!(attach_availability_change_event: 23, [(3, 0, 0) =>]),
            ipc_interface_make_command_meta!(recreate_application_area: 24, [(3, 0, 0) =>])
        ]
    }
}

impl IUser for User {
    fn initialize(&mut self, aruid: applet::AppletResourceUserId, process_id: sf::ProcessId, mcu_data: sf::InMapAliasBuffer) -> Result<()> {
        // TODO: make use of mcu data?
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "aruid: 0x{:X}, process_id: 0x{:X}, mcu data: {:p} - {}\n", aruid, process_id.process_id, mcu_data.buf, mcu_data.size);
        result_return_unless!(self.is_state(nfp::State::NonInitialized), results::nfp::ResultDeviceNotFound);

        self.state.set(nfp::State::Initialized);
        self.device_state.set(nfp::DeviceState::Initialized);
        
        self.activate_event = wait::SystemEvent::new()?;
        self.deactivate_event = wait::SystemEvent::new()?;
        self.availability_change_event = wait::SystemEvent::new()?;

        self.emu_handler_thread = thread::Thread::new(Self::emu_handler_impl, self as *mut Self as *mut u8, core::ptr::null_mut(), 0x1000, "emuiibo.AmiiboEmulationHandler")?;
        self.emu_handler_thread.create_and_start(0x2B, -2)?;
        Ok(())
    }

    fn finalize(&mut self) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        
        self.state.set(nfp::State::NonInitialized);
        self.device_state.set(nfp::DeviceState::Finalized);
        Ok(())
    }

    fn list_devices(&mut self, out_devices: sf::OutPointerBuffer) -> Result<u32> {
        let mut devices: &mut [nfp::DeviceHandle] = out_devices.get_mut_slice();
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        
        // Send a single fake device handle
        // TODO: use hid to detect if the joycons are attached/detached
        // Meanwhile, hardcode handheld
        devices[0].npad_id = 0x20;
        Ok(1)
    }

    fn start_detection(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::Initialized) || self.is_device_state(nfp::DeviceState::TagRemoved), results::nfp::ResultDeviceNotFound);
        
        self.device_state.set(nfp::DeviceState::SearchingForTag);
        Ok(())
    }

    fn stop_detection(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);

        self.device_state.set(nfp::DeviceState::Initialized);
        Ok(())
    }

    fn mount(&mut self, _device_handle: nfp::DeviceHandle, device_type: nfp::DeviceType, mount_target: nfp::MountTarget) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "mount... device type: {:?}, mount target: {:?}\n", device_type, mount_target);
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        
        self.device_state.set(nfp::DeviceState::TagMounted);
        Ok(())
    }

    fn unmount(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "unmount...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        
        self.device_state.set(nfp::DeviceState::TagFound);
        Ok(())
    }

    fn open_application_area(&mut self, _device_handle: nfp::DeviceHandle, access_id: nfp::AccessId) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "open_application_area...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);

        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let application_area = area::ApplicationArea::from(&amiibo, access_id);
        result_return_unless!(application_area.exists(), results::nfp::ResultAreaNeedsToBeCreated);

        self.current_opened_area = application_area;
        Ok(())
    }

    fn get_application_area(&mut self, _device_handle: nfp::DeviceHandle, out_data: sf::OutMapAliasBuffer) -> Result<u32> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_application_area...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.current_opened_area.exists(), results::nfp::ResultAreaNeedsToBeCreated);

        let area_size = self.current_opened_area.get_size()?;
        let size = core::cmp::min(area_size, out_data.size);
        
        self.current_opened_area.read(out_data.buf as *mut u8, size)?;
        Ok(size as u32)
    }

    fn set_application_area(&mut self, _device_handle: nfp::DeviceHandle, data: sf::InMapAliasBuffer) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "set_application_area...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.current_opened_area.exists(), results::nfp::ResultAreaNeedsToBeCreated);

        let area_size = self.current_opened_area.get_size()?;
        let size = core::cmp::min(area_size, data.size);

        self.current_opened_area.write(data.buf, size)?;
        let amiibo = emu::get_active_virtual_amiibo();
        amiibo.notify_written()?;
        Ok(())
    }

    fn flush(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "flush...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);

        Ok(())
    }

    fn restore(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "restore...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);

        Ok(())
    }

    fn create_application_area(&mut self, _device_handle: nfp::DeviceHandle, access_id: nfp::AccessId, data: sf::InMapAliasBuffer) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "create_application_area...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);

        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let application_area = area::ApplicationArea::from(&amiibo, access_id);
        result_return_if!(application_area.exists(), results::nfp::ResultAreaNeedsToBeCreated);

        application_area.create(data.buf, data.size, false)?;
        amiibo.notify_written()?;
        Ok(())
    }

    fn get_tag_info(&mut self, _device_handle: nfp::DeviceHandle, mut out_tag_info: sf::OutFixedPointerBuffer<nfp::TagInfo>) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Tag info...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagFound) || self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let tag_info = amiibo.produce_tag_info()?;
        out_tag_info.set_as(tag_info);
        Ok(())
    }

    fn get_register_info(&mut self, _device_handle: nfp::DeviceHandle, mut out_register_info: sf::OutFixedPointerBuffer<nfp::RegisterInfo>) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Register info...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let register_info = amiibo.produce_register_info()?;
        out_register_info.set_as(register_info);
        Ok(())
    }

    fn get_common_info(&mut self, _device_handle: nfp::DeviceHandle, mut out_common_info: sf::OutFixedPointerBuffer<nfp::CommonInfo>) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Common info...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let common_info = amiibo.produce_common_info()?;
        out_common_info.set_as(common_info);
        Ok(())
    }

    fn get_model_info(&mut self, _device_handle: nfp::DeviceHandle, mut out_model_info: sf::OutFixedPointerBuffer<nfp::ModelInfo>) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Model info...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let model_info = amiibo.produce_model_info()?;
        out_model_info.set_as(model_info);
        Ok(())
    }

    fn attach_activate_event(&mut self, _device_handle: nfp::DeviceHandle) -> Result<sf::CopyHandle> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "attach_activate_event...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        
        Ok(sf::Handle::from(self.activate_event.client_handle))
    }

    fn attach_deactivate_event(&mut self, _device_handle: nfp::DeviceHandle) -> Result<sf::CopyHandle> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "attach_deactivate_event...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        
        Ok(sf::Handle::from(self.deactivate_event.client_handle))
    }

    fn get_state(&mut self) -> Result<nfp::State> {
        Ok(self.state.get_val())
    }

    fn get_device_state(&mut self, _device_handle: nfp::DeviceHandle) -> Result<nfp::DeviceState> {
        Ok(self.device_state.get_val())
    }

    fn get_npad_id(&mut self, device_handle: nfp::DeviceHandle) -> Result<u32> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_npad_id...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        
        Ok(device_handle.npad_id)
    }

    fn get_application_area_size(&mut self, _device_handle: nfp::DeviceHandle) -> Result<u32> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_application_area_size...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.current_opened_area.exists(), results::nfp::ResultAreaNeedsToBeCreated);

        let area_size = self.current_opened_area.get_size()?;
        Ok(area_size as u32)
    }

    fn attach_availability_change_event(&mut self) -> Result<sf::CopyHandle> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "attach_availability_change_event...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        
        Ok(sf::Handle::from(self.availability_change_event.client_handle))
    }

    fn recreate_application_area(&mut self, _device_handle: nfp::DeviceHandle, access_id: nfp::AccessId, data: sf::InMapAliasBuffer) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "recreate_application_area...\n");
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);

        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let application_area = area::ApplicationArea::from(&amiibo, access_id);
        application_area.create(data.buf, data.size, true)?;
        amiibo.notify_written()?;
        Ok(())
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
            ipc_interface_make_command_meta!(create_user_interface: 0)
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
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "create_user_interface...\n");
        Ok(mem::Shared::new(User::new(self.info.program_id)))
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