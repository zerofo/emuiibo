use nx::result::*;
use nx::results;
use nx::mem;
use nx::ipc::sf;
use nx::ipc::server;
use nx::ipc::sf::applet;
use nx::ipc::sf::nfp;
use nx::ipc::sf::nfp::IUser;
use nx::ipc::sf::nfp::IUserManager;
use nx::ipc::sf::nfp::ISystem;
use nx::ipc::sf::nfp::ISystemManager;
use nx::ipc::sf::sm;
use nx::wait;
use nx::sync;
use nx::service::hid;
use nx::input;
use nx::thread;

use crate::area;
use crate::emu;
use crate::logger;

pub struct System {
    session: sf::Session,
    application_id: u64,
    activate_event: wait::SystemEvent,
    deactivate_event: wait::SystemEvent,
    availability_change_event: wait::SystemEvent,
    state: sync::Locked<nfp::State>,
    device_state: sync::Locked<nfp::DeviceState>,
    should_end_thread: sync::Locked<bool>,
    current_opened_area: area::ApplicationArea,
    emu_handler_thread: thread::Thread,
    input_ctx: input::InputContext
}

impl System {
    pub fn new(application_id: u64) -> Result<Self> {
        // emu::register_intercepted_application_id(application_id);
        let supported_tags = hid::NpadStyleTag::ProController() | hid::NpadStyleTag::Handheld() | hid::NpadStyleTag::JoyconPair() | hid::NpadStyleTag::JoyconLeft() | hid::NpadStyleTag::JoyconRight() | hid::NpadStyleTag::SystemExt() | hid::NpadStyleTag::System();
        let controllers = [hid::ControllerId::Player1, hid::ControllerId::Handheld];
        let input_ctx = input::InputContext::new(0, supported_tags, &controllers)?;
        Ok(Self { session: sf::Session::new(), application_id: application_id, activate_event: wait::SystemEvent::empty(), deactivate_event: wait::SystemEvent::empty(), availability_change_event: wait::SystemEvent::empty(), state: sync::Locked::new(false, nfp::State::NonInitialized), device_state: sync::Locked::new(false, nfp::DeviceState::Unavailable), should_end_thread: sync::Locked::new(false, false), emu_handler_thread: thread::Thread::empty(), current_opened_area: area::ApplicationArea::new(), input_ctx: input_ctx })
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

    fn emu_handler_thread_fn(system: &*mut System) {
        unsafe {
            loop {
                if (*(*system)).should_end_thread.get_val() {
                    break;
                }
                
                let status = emu::get_active_virtual_amiibo_status();
                (*(*system)).handle_virtual_amiibo_status(status);
                let _ = thread::sleep(100_000_000);
            }
        }
    }
}

impl Drop for System {
    fn drop(&mut self) {
        // emu::unregister_intercepted_application_id(self.application_id);
        self.should_end_thread.set(true);
        self.emu_handler_thread.join().unwrap();
    }
}

impl sf::IObject for System {
    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.session
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
    fn initialize_system(&mut self, _aruid: applet::AppletResourceUserId, _process_id: sf::ProcessId, _mcu_data: sf::InMapAliasBuffer) -> Result<()> {
        // TODO: make use of aruid or mcu data?
        result_return_unless!(self.is_state(nfp::State::NonInitialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!InitializeSystem");

        self.state.set(nfp::State::Initialized);
        self.device_state.set(nfp::DeviceState::Initialized);
        
        self.activate_event = wait::SystemEvent::new()?;
        self.deactivate_event = wait::SystemEvent::new()?;
        self.availability_change_event = wait::SystemEvent::new()?;

        self.emu_handler_thread = thread::Thread::new(Self::emu_handler_thread_fn, &(self as *mut Self), "emuiibo.AmiiboEmulationHandlerS", 0x1000)?;
        self.emu_handler_thread.initialize(0x2B, -2)?;
        self.emu_handler_thread.start()?;
        Ok(())
    }

    fn finalize_system(&mut self) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!FinalizeSystem");

        self.state.set(nfp::State::NonInitialized);
        self.device_state.set(nfp::DeviceState::Finalized);
        Ok(())
    }

    fn list_devices(&mut self, out_devices: sf::OutPointerBuffer) -> Result<u32> {
        let mut devices: &mut [nfp::DeviceHandle] = out_devices.get_mut_slice();
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!ListDevices");

        // Send a single fake device handle
        devices[0].id = match self.input_ctx.is_controller_connected(hid::ControllerId::Player1) {
            true => hid::ControllerId::Player1,
            false => hid::ControllerId::Handheld
        } as u32;
        Ok(1)
    }

    fn start_detection(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::Initialized) || self.is_device_state(nfp::DeviceState::TagRemoved), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!StartDetection");

        self.device_state.set(nfp::DeviceState::SearchingForTag);
        Ok(())
    }

    fn stop_detection(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!StopDetection");

        self.device_state.set(nfp::DeviceState::Initialized);
        Ok(())
    }

    fn mount(&mut self, _device_handle: nfp::DeviceHandle, _model_type: nfp::ModelType, _mount_target: nfp::MountTarget) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!Mount");
        
        self.device_state.set(nfp::DeviceState::TagMounted);
        Ok(())
    }

    fn unmount(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!Unmount");
        
        self.device_state.set(nfp::DeviceState::TagFound);
        Ok(())
    }

    fn flush(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!Flush");

        Ok(())
    }

    fn restore(&mut self, _device_handle: nfp::DeviceHandle) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!Restore");

        Ok(())
    }

    fn get_tag_info(&mut self, _device_handle: nfp::DeviceHandle, mut out_tag_info: sf::OutFixedPointerBuffer<nfp::TagInfo>) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagFound) || self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!GetTagInfo");

        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let tag_info = amiibo.produce_tag_info()?;
        out_tag_info.set_as(tag_info);
        Ok(())
    }

    fn get_register_info(&mut self, _device_handle: nfp::DeviceHandle, mut out_register_info: sf::OutFixedPointerBuffer<nfp::RegisterInfo>) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!GetRegisterInfo");

        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let register_info = amiibo.produce_register_info()?;
        out_register_info.set_as(register_info);
        Ok(())
    }

    fn get_common_info(&mut self, _device_handle: nfp::DeviceHandle, mut out_common_info: sf::OutFixedPointerBuffer<nfp::CommonInfo>) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!GetCommonInfo");

        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let common_info = amiibo.produce_common_info()?;
        out_common_info.set_as(common_info);
        Ok(())
    }

    fn get_model_info(&mut self, _device_handle: nfp::DeviceHandle, mut out_model_info: sf::OutFixedPointerBuffer<nfp::ModelInfo>) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!GetModelInfo");

        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let model_info = amiibo.produce_model_info()?;
        out_model_info.set_as(model_info);
        Ok(())
    }

    fn attach_activate_event(&mut self, _device_handle: nfp::DeviceHandle) -> Result<sf::CopyHandle> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!AttachActivateEvent");

        Ok(sf::Handle::from(self.activate_event.client_handle))
    }

    fn attach_deactivate_event(&mut self, _device_handle: nfp::DeviceHandle) -> Result<sf::CopyHandle> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!AttachDeactivateEvent");

        Ok(sf::Handle::from(self.deactivate_event.client_handle))
    }

    fn get_state(&mut self) -> Result<nfp::State> {
        logger::log_line_str("nfp:sys!GetState");
        Ok(self.state.get_val())
    }

    fn get_device_state(&mut self, _device_handle: nfp::DeviceHandle) -> Result<nfp::DeviceState> {
        logger::log_line_str("nfp:sys!GetDeviceState");
        Ok(self.device_state.get_val())
    }

    fn get_npad_id(&mut self, device_handle: nfp::DeviceHandle) -> Result<u32> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!GetNpadId");
        
        Ok(device_handle.id)
    }

    fn attach_availability_change_event(&mut self) -> Result<sf::CopyHandle> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!AttachAvChangeEvent");

        Ok(sf::Handle::from(self.availability_change_event.client_handle))
    }

    fn format(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        logger::log_line_str("nfp:sys!Format");

        Ok(())
    }

    fn get_admin_info(&mut self, device_handle: nfp::DeviceHandle, mut out_admin_info: sf::OutFixedPointerBuffer<nfp::AdminInfo>) -> Result<()> {
        logger::log_line_str("nfp:sys!GetAdminInfo");

        /*
        Amiibo settings flags:
        0x10 / bit-4: initialized with console settings
        0x20 / bit-5: saved appdata (used by game) --> last write date

        0x01006A800016E000
        0x00040000000EE000

        unk_8_2: 0 if 3DS title, 1 if wiiu, 2 3ds again, 3 if switch title
        */
        
        let mut info: nfp::AdminInfo = unsafe { core::mem::zeroed() };
        info.program_id = 0x0005000C101D7500;
        info.flags = nfp::AdminInfoFlags::IsInitialized() | nfp::AdminInfoFlags::HasApplicationArea();
        info.crc32_change_counter = 20;
        info.access_id = 0x34F80200;
        info.unk_0x2 = 0x2;
        info.console_type = nfp::ProgramIdConsoleType::NintendoWiiU;

        out_admin_info.set_as(info);
        Ok(())
    }

    fn get_register_info_private(&mut self, device_handle: nfp::DeviceHandle, mut out_register_info_private: sf::OutFixedPointerBuffer<nfp::RegisterInfoPrivate>) -> Result<()> {
        result_return_unless!(self.is_state(nfp::State::Initialized), results::nfp::ResultDeviceNotFound);
        result_return_unless!(self.is_device_state(nfp::DeviceState::TagMounted), results::nfp::ResultDeviceNotFound);
        logger::log_line_str("nfp:sys!GetRegisterInfoPrivate");

        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), results::nfp::ResultDeviceNotFound);

        let register_info = amiibo.produce_register_info()?;
        let mut register_info_private = nfp::RegisterInfoPrivate {
            mii_store_data: unsafe { core::mem::zeroed() },
            first_write_date: register_info.first_write_date,
            name: register_info.name,
            unk: register_info.unk,
            reserved: [0; 0x8E]
        };

        out_register_info_private.set_as(register_info_private);
        Ok(())
    }

    fn set_register_info_private(&mut self, device_handle: nfp::DeviceHandle, register_info_private: sf::InFixedPointerBuffer<nfp::RegisterInfoPrivate>) -> Result<()> {
        logger::log_line_str("nfp:sys!SetRegisterInfoPrivate");

        Ok(())
    }

    fn delete_register_info(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        logger::log_line_str("nfp:sys!DeleteRegisterInfo");

        Ok(())
    }

    fn delete_application_area(&mut self, device_handle: nfp::DeviceHandle) -> Result<()> {
        logger::log_line_str("nfp:sys!DeleteApplicationArea");

        Ok(())
    }

    fn exists_application_area(&mut self, device_handle: nfp::DeviceHandle) -> Result<bool> {
        logger::log_line_str("nfp:sys!ExistsApplicationArea");

        Ok(true)
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