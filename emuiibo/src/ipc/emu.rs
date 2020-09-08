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
use alloc::string::String;

use crate::emu;
use crate::amiibo;
use crate::fsext;

pub trait IEmulationService {
    ipc_interface_define_command!(get_version: () => (version: emu::Version));
    ipc_interface_define_command!(get_virtual_amiibo_directory: (out_path: sf::OutMapAliasBuffer) => ());
    ipc_interface_define_command!(get_emulation_status: () => (status: emu::EmulationStatus));
    ipc_interface_define_command!(set_emulation_status: (status: emu::EmulationStatus) => ());
    ipc_interface_define_command!(get_active_virtual_amiibo: (out_path: sf::OutMapAliasBuffer) => (virtual_amiibo: amiibo::VirtualAmiiboData));
    ipc_interface_define_command!(set_active_virtual_amiibo: (path: sf::InMapAliasBuffer) => ());
    ipc_interface_define_command!(reset_active_virtual_amiibo: () => ());
    ipc_interface_define_command!(get_active_virtual_amiibo_status: () => (status: emu::VirtualAmiiboStatus));
    ipc_interface_define_command!(set_active_virtual_amiibo_status: (status: emu::VirtualAmiiboStatus) => ());
    ipc_interface_define_command!(is_application_id_intercepted: (application_id: u64) => (is_intercepted: bool));
    ipc_interface_define_command!(is_current_application_id_intercepted: () => (is_intercepted: bool));
    ipc_interface_define_command!(try_parse_virtual_amiibo: (path: sf::InMapAliasBuffer) => (virtual_amiibo: amiibo::VirtualAmiiboData));
}

pub struct EmulationService {
    session: sf::Session
}

impl sf::IObject for EmulationService {
    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.session
    }

    fn get_command_table(&self) -> sf::CommandMetadataTable {
        ipc_server_make_command_table! {
            get_version: 0,
            get_virtual_amiibo_directory: 1,
            get_emulation_status: 2,
            set_emulation_status: 3,
            get_active_virtual_amiibo: 4,
            set_active_virtual_amiibo: 5,
            reset_active_virtual_amiibo: 6,
            get_active_virtual_amiibo_status: 7,
            set_active_virtual_amiibo_status: 8,
            is_application_id_intercepted: 9,
            is_current_application_id_intercepted: 10,
            try_parse_virtual_amiibo: 11
        }
    }
}

impl server::IServerObject for EmulationService {
    fn new() -> Self {
        Self { session: sf::Session::new() }
    }
}

impl IEmulationService for EmulationService {
    fn get_version(&mut self) -> Result<emu::Version> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_version... version: {}.{}.{} (dev: {})", emu::CURRENT_VERSION.major, emu::CURRENT_VERSION.minor, emu::CURRENT_VERSION.micro, emu::CURRENT_VERSION.is_dev_build);
        Ok(emu::CURRENT_VERSION)
    }

    fn get_virtual_amiibo_directory(&mut self, mut out_path: sf::OutMapAliasBuffer) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_virtual_amiibo_directory... dir: {}", fsext::VIRTUAL_AMIIBO_DIR);
        out_path.set_string(String::from(fsext::VIRTUAL_AMIIBO_DIR));
        Ok(())
    }

    fn get_emulation_status(&mut self) -> Result<emu::EmulationStatus> {
        let status = emu::get_emulation_status();
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_emulation_status... status: {:?}", status);
        Ok(status)
    }

    fn set_emulation_status(&mut self, status: emu::EmulationStatus) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "set_emulation_status... new status: {:?}", status);
        emu::set_emulation_status(status);
        Ok(())
    }

    fn get_active_virtual_amiibo(&mut self, mut out_path: sf::OutMapAliasBuffer) -> Result<amiibo::VirtualAmiiboData> {
        let amiibo = emu::get_active_virtual_amiibo();
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_active_virtual_amiibo... path: {}", amiibo.path);
        result_return_unless!(amiibo.is_valid(), 0x360);

        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => " - amiibo: {:?}", amiibo.info);
        let data = amiibo.produce_data()?;
        
        out_path.set_string(amiibo.path.clone());
        Ok(data)
    }

    fn set_active_virtual_amiibo(&mut self, path: sf::InMapAliasBuffer) -> Result<()> {
        let path_str = path.get_string();
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "set_active_virtual_amiibo... path: {}", path_str);
        let amiibo = amiibo::try_load_virtual_amiibo(path_str)?;
        result_return_unless!(amiibo.is_valid(), 0x360);

        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => " - amiibo: {:?}", amiibo.info);
        emu::set_active_virtual_amiibo(amiibo);
        Ok(())
    }

    fn reset_active_virtual_amiibo(&mut self) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "reset_active_virtual_amiibo...");
        emu::set_active_virtual_amiibo(amiibo::VirtualAmiibo::empty());
        Ok(())
    }

    fn get_active_virtual_amiibo_status(&mut self) -> Result<emu::VirtualAmiiboStatus> {
        let status = emu::get_active_virtual_amiibo_status();
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "get_active_virtual_amiibo_status... status: {:?}", status);
        Ok(status)
    }

    fn set_active_virtual_amiibo_status(&mut self, status: emu::VirtualAmiiboStatus) -> Result<()> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "set_active_virtual_amiibo_status... status: {:?}", status);
        emu::set_active_virtual_amiibo_status(status);
        Ok(())
    }

    fn is_application_id_intercepted(&mut self, application_id: u64) -> Result<bool> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "is_application_id_intercepted...");
        Ok(emu::is_application_id_intercepted(application_id))
    }

    fn is_current_application_id_intercepted(&mut self) -> Result<bool> {
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "is_current_application_id_intercepted...");
        // TODO
        Ok(false)
    }

    fn try_parse_virtual_amiibo(&mut self, path: sf::InMapAliasBuffer) -> Result<amiibo::VirtualAmiiboData> {
        let path_str = path.get_string();
        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => "try_parse_virtual_amiibo - path: {}", path_str);
        let amiibo = amiibo::try_load_virtual_amiibo(path_str)?;
        result_return_unless!(amiibo.is_valid(), 0x360);

        // diag_log!(log::LmLogger { log::LogSeverity::Error, true } => " - amiibo: {:?}", amiibo.info);
        let data = amiibo.produce_data()?;
        Ok(data)
    }
}

impl server::IService for EmulationService {
    fn get_name() -> &'static str {
        nul!("emuiibo")
    }

    fn get_max_sesssions() -> i32 {
        40
    }
}