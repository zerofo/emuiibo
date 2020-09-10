use nx::result::*;
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

use crate::emu;

pub struct User {
    session: sf::Session,
    activate_event: wait::SystemEvent,
    deactivate_event: wait::SystemEvent,
    availability_change_event: wait::SystemEvent,
    state: sync::Locked<nfp::State>,
    device_state: sync::Locked<nfp::DeviceState>,
    should_end_thread: sync::Locked<bool>,
    emu_handler_thread: thread::Thread
}

impl User {
    pub fn new() -> Self {
        Self { session: sf::Session::new(), activate_event: wait::SystemEvent::empty(), deactivate_event: wait::SystemEvent::empty(), availability_change_event: wait::SystemEvent::empty(), state: sync::Locked::new(false, nfp::State::NonInitialized), device_state: sync::Locked::new(false, nfp::DeviceState::Unavailable), emu_handler_thread: thread::Thread::empty(), should_end_thread: sync::Locked::new(false, false) }
    }

    pub fn is_state(&mut self, state: nfp::State) -> bool {
        *self.state.get() == state
    }

    pub fn is_device_state(&mut self, device_state: nfp::DeviceState) -> bool {
        *self.device_state.get() == device_state
    }

    pub fn handle_virtual_amiibo_status(&mut self, status: emu::VirtualAmiiboStatus) {
        match status {
            emu::VirtualAmiiboStatus::Connected => match *self.device_state.get() {
                nfp::DeviceState::SearchingForTag => {
                    diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Connected: SearchingForTag => TagFound");
                    self.device_state.set(nfp::DeviceState::TagFound);
                    self.activate_event.signal().unwrap();
                },
                _ => {}
            },
            emu::VirtualAmiiboStatus::Disconnected => match *self.device_state.get() {
                nfp::DeviceState::TagFound | nfp::DeviceState::TagMounted => {
                    diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Disconnected: TagFound/TagMounted => SearchingForTag");
                    self.device_state.set(nfp::DeviceState::SearchingForTag);
                    self.deactivate_event.signal().unwrap();
                },
                _ => {}
            },
            _ => {}
        };
    }

    fn emu_handler_impl(user_v: *mut u8) {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Starting emu_handler thread...");
        let user = user_v as *mut User;
        unsafe {
            loop {
                if *(*user).should_end_thread.get() {
                    break;
                }
                
                let status = emu::get_active_virtual_amiibo_status();
                (*user).handle_virtual_amiibo_status(status);
                let _ = thread::sleep(100_000_000);
            }
        }
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Exiting emu_handler thread...");
    }
}

impl Drop for User {
    fn drop(&mut self) {
        // TODO: emu::unregister_intercepted_application_id
        self.should_end_thread.set(true);
        self.emu_handler_thread.join().unwrap();
    }
}

impl sf::IObject for User {
    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.session
    }

    fn get_command_table(&self) -> sf::CommandMetadataTable {
        ipc_server_make_command_table! {
            initialize: 0,
            finalize: 1,
            list_devices: 2,
            start_detection: 3,
            stop_detection: 4,
            mount: 5,
            unmount: 6,
            open_application_area: 7,
            get_application_area: 8,
            set_application_area: 9,
            flush: 10,
            restore: 11,
            create_application_area: 12,
            get_tag_info: 13,
            get_register_info: 14,
            get_common_info: 15,
            get_model_info: 16,
            attach_activate_event: 17,
            attach_deactivate_event: 18,
            get_state: 19,
            get_device_state: 20,
            get_npad_id: 21,
            get_application_area_size: 22,
            attach_availability_change_event: 23,
            recreate_application_area: 24
        }
    }
}

impl IUser for User {
    fn initialize(&mut self, aruid: applet::AppletResourceUserId, process_id: sf::ProcessId, mcu_data: sf::InMapAliasBuffer) -> Result<()> {
        // TODO: make use of mcu data?
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "aruid: 0x{:X}, process_id: 0x{:X}, mcu data: {:p} - {}", aruid, process_id.process_id, mcu_data.buf, mcu_data.size);
        result_return_unless!(self.is_state(nfp::State::NonInitialized), 0x8073);

        self.state.set(nfp::State::Initialized);
        self.device_state.set(nfp::DeviceState::Initialized);
        // TODO: emu::register_intercepted_application_id
        
        self.activate_event = wait::SystemEvent::new()?;
        self.deactivate_event = wait::SystemEvent::new()?;
        self.availability_change_event = wait::SystemEvent::new()?;

        self.emu_handler_thread = thread::Thread::new(Self::emu_handler_impl, self as *mut Self as *mut u8, core::ptr::null_mut(), 0x1000, "emuiibo.AmiiboEmulationHandler")?;
        self.emu_handler_thread.create_and_start(0x2B, -2)?;
        
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Everything initialized!");
        Ok(())
    }

    fn finalize(&mut self) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Finalizing...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        
        self.state.set(nfp::State::NonInitialized);
        self.device_state.set(nfp::DeviceState::Finalized);
        Ok(())
    }

    fn list_devices(&mut self, out_devices: sf::OutPointerBuffer) -> Result<u32> {
        let mut devices: &mut [nfp::DeviceHandle] = out_devices.get_mut_slice();
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Array length: {}", devices.len());
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        
        // Send a single fake device handle
        // TODO: use hid to detect if the joycons are attached/detached
        // Meanwhile, hardcode handheld
        devices[0].npad_id = 0x20;
        Ok(1)
    }

    fn start_detection(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Start detection...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        result_return_unless!(self.is_device_state(nfp::DeviceState::Initialized) || self.is_device_state(nfp::DeviceState::TagRemoved), 0x8073);
        
        self.device_state.set(nfp::DeviceState::SearchingForTag);
        Ok(())
    }

    fn stop_detection(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Stop detection...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);

        self.device_state.set(nfp::DeviceState::Initialized);
        Ok(())
    }

    fn mount(&mut self, device_handle: nfp::DeviceHandle, device_type: nfp::DeviceType, mount_target: nfp::MountTarget) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "mount...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        
        self.device_state.set(nfp::DeviceState::TagMounted);
        Ok(())
    }

    fn unmount(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "unmount...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        
        self.device_state.set(nfp::DeviceState::TagFound);
        Ok(())
    }

    fn open_application_area(&mut self, device_handle: nfp::DeviceHandle, access_id: nfp::AccessId) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "open_application_area...");
        
        // TODO
        Err(ResultCode::new(0x10073))
    }

    fn get_application_area(&mut self, device_handle: nfp::DeviceHandle, out_data: sf::OutMapAliasBuffer) -> Result<u32> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_application_area...");

        // TODO
        Ok(out_data.size as u32)
    }

    fn set_application_area(&mut self, device_handle: nfp::DeviceHandle, data: sf::InMapAliasBuffer) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "set_application_area...");
        
        // TODO
        Ok(())
    }

    fn flush(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "flush...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);

        Ok(())
    }

    fn restore(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "restore...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);

        Ok(())
    }

    fn create_application_area(&mut self, device_handle: nfp::DeviceHandle, access_id: nfp::AccessId, data: sf::InMapAliasBuffer) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "create_application_area...");
        
        // TODO
        Ok(())
    }

    fn get_tag_info(&mut self, device_handle: nfp::DeviceHandle, mut out_tag_info: sf::OutFixedPointerBuffer<nfp::TagInfo>) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Tag info...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagFound) || self.is_device_state(nfp::DeviceState::TagMounted), 0x8073);
        
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), 0x8073);

        out_tag_info.set_as(amiibo.produce_tag_info());
        Ok(())
    }

    fn get_register_info(&mut self, device_handle: nfp::DeviceHandle, mut out_register_info: sf::OutFixedPointerBuffer<nfp::RegisterInfo>) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Register info...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), 0x8073);
        
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), 0x8073);

        out_register_info.set_as(amiibo.produce_register_info());
        Ok(())
    }

    fn get_common_info(&mut self, device_handle: nfp::DeviceHandle, mut out_common_info: sf::OutFixedPointerBuffer<nfp::CommonInfo>) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Common info...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), 0x8073);
        
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), 0x8073);

        out_common_info.set_as(amiibo.produce_common_info());
        Ok(())
    }

    fn get_model_info(&mut self, device_handle: nfp::DeviceHandle, mut out_model_info: sf::OutFixedPointerBuffer<nfp::ModelInfo>) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "Model info...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), 0x8073);
        
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), 0x8073);

        out_model_info.set_as(amiibo.produce_model_info());
        Ok(())
    }

    fn attach_activate_event(&mut self, device_handle: nfp::DeviceHandle) -> Result<sf::CopyHandle> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "attach_activate_event...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        
        Ok(sf::Handle::from(self.activate_event.client_handle))
    }

    fn attach_deactivate_event(&mut self, device_handle: nfp::DeviceHandle) -> Result<sf::CopyHandle> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "attach_deactivate_event...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        
        Ok(sf::Handle::from(self.deactivate_event.client_handle))
    }

    fn get_state(&mut self) -> Result<nfp::State> {
        let state = *self.state.get();
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_state... - state: {:?}", state);
        Ok(state)
    }

    fn get_device_state(&mut self, device_handle: nfp::DeviceHandle) -> Result<nfp::DeviceState> {
        let device_state = *self.device_state.get();
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_device_state... - device state: {:?}", device_state);
        Ok(device_state)
    }

    fn get_npad_id(&mut self, device_handle: nfp::DeviceHandle) -> Result<u32> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_npad_id...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        
        Ok(device_handle.npad_id)
    }

    fn get_application_area_size(&mut self, device_handle: nfp::DeviceHandle) -> Result<u32> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_application_area_size...");
        
        // TODO
        Ok(0)
    }

    fn attach_availability_change_event(&mut self) -> Result<sf::CopyHandle> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "attach_availability_change_event...");
        result_return_unless!(self.is_state(nfp::State::Initialized), 0x8073);
        
        Ok(sf::Handle::from(self.availability_change_event.client_handle))
    }

    fn recreate_application_area(&mut self, device_handle: nfp::DeviceHandle, access_id: nfp::AccessId, data: sf::InMapAliasBuffer) -> Result<()> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "recreate_application_area...");
        
        // TODO
        Ok(())
    }
}

pub struct UserManager {
    session: sf::Session
}

impl sf::IObject for UserManager {
    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.session
    }

    fn get_command_table(&self) -> sf::CommandMetadataTable {
        ipc_server_make_command_table! {
            create_user_interface: 0
        }
    }
}

impl server::IServerObject for UserManager {
    fn new() -> Self {
        Self { session: sf::Session::new() }
    }
}

impl IUserManager for UserManager {
    fn create_user_interface(&mut self) -> Result<mem::Shared<dyn sf::IObject>> {
        diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "create_user_interface...");
        Ok(mem::Shared::new(User::new()))
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