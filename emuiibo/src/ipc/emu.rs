use nx::result::*;
use nx::ipc::sf;
use nx::ipc::server;
use nx::ipc::sf::nfp;
use alloc::string::String;

use crate::resultsext;
use crate::emu;
use crate::amiibo;
use crate::fsext;

pub trait IEmulationService {
    ipc_cmif_interface_define_command!(get_version: () => (version: emu::Version));
    ipc_cmif_interface_define_command!(get_virtual_amiibo_directory: (out_path: sf::OutMapAliasBuffer) => ());
    ipc_cmif_interface_define_command!(get_emulation_status: () => (status: emu::EmulationStatus));
    ipc_cmif_interface_define_command!(set_emulation_status: (status: emu::EmulationStatus) => ());
    ipc_cmif_interface_define_command!(get_active_virtual_amiibo: (out_path: sf::OutMapAliasBuffer) => (virtual_amiibo: amiibo::VirtualAmiiboData));
    ipc_cmif_interface_define_command!(set_active_virtual_amiibo: (path: sf::InMapAliasBuffer) => ());
    ipc_cmif_interface_define_command!(reset_active_virtual_amiibo: () => ());
    ipc_cmif_interface_define_command!(get_active_virtual_amiibo_status: () => (status: emu::VirtualAmiiboStatus));
    ipc_cmif_interface_define_command!(set_active_virtual_amiibo_status: (status: emu::VirtualAmiiboStatus) => ());
    ipc_cmif_interface_define_command!(is_application_id_intercepted: (application_id: u64) => (is_intercepted: bool));
    ipc_cmif_interface_define_command!(try_parse_virtual_amiibo: (path: sf::InMapAliasBuffer) => (virtual_amiibo: amiibo::VirtualAmiiboData));
    ipc_cmif_interface_define_command!(get_active_virtual_amiibo_areas: (out_areas: sf::OutMapAliasBuffer) => (count: u32));
    ipc_cmif_interface_define_command!(get_active_virtual_amiibo_current_area: () => (access_id: nfp::AccessId));
    ipc_cmif_interface_define_command!(set_active_virtual_amiibo_current_area: (access_id: nfp::AccessId) => ());
}

pub struct EmulationService {
    session: sf::Session
}

impl sf::IObject for EmulationService {
    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.session
    }

    fn get_command_table(&self) -> sf::CommandMetadataTable {
        vec! [
            ipc_cmif_interface_make_command_meta!(get_version: 0),
            ipc_cmif_interface_make_command_meta!(get_virtual_amiibo_directory: 1),
            ipc_cmif_interface_make_command_meta!(get_emulation_status: 2),
            ipc_cmif_interface_make_command_meta!(set_emulation_status: 3),
            ipc_cmif_interface_make_command_meta!(get_active_virtual_amiibo: 4),
            ipc_cmif_interface_make_command_meta!(set_active_virtual_amiibo: 5),
            ipc_cmif_interface_make_command_meta!(reset_active_virtual_amiibo: 6),
            ipc_cmif_interface_make_command_meta!(get_active_virtual_amiibo_status: 7),
            ipc_cmif_interface_make_command_meta!(set_active_virtual_amiibo_status: 8),
            ipc_cmif_interface_make_command_meta!(is_application_id_intercepted: 9),
            ipc_cmif_interface_make_command_meta!(try_parse_virtual_amiibo: 10),
            ipc_cmif_interface_make_command_meta!(get_active_virtual_amiibo_areas: 11),
            ipc_cmif_interface_make_command_meta!(get_active_virtual_amiibo_current_area: 12),
            ipc_cmif_interface_make_command_meta!(set_active_virtual_amiibo_current_area: 13)
        ]
    }
}

impl server::IServerObject for EmulationService {
    fn new() -> Self {
        Self { session: sf::Session::new() }
    }
}

impl IEmulationService for EmulationService {
    fn get_version(&mut self) -> Result<emu::Version> {
        log!("\n[emu] GetVersion -- (...)\n");
        Ok(emu::CURRENT_VERSION)
    }

    fn get_virtual_amiibo_directory(&mut self, mut out_path: sf::OutMapAliasBuffer) -> Result<()> {
        log!("\n[emu] GetVirtualAmiiboDirectory -- (...)\n");
        out_path.set_string(String::from(fsext::VIRTUAL_AMIIBO_DIR));
        Ok(())
    }

    fn get_emulation_status(&mut self) -> Result<emu::EmulationStatus> {
        log!("\n[emu] GetEmulationStatus -- (...)\n");
        let status = emu::get_emulation_status();
        Ok(status)
    }

    fn set_emulation_status(&mut self, status: emu::EmulationStatus) -> Result<()> {
        log!("\n[emu] SetEmulationStatus -- status: {:?}\n", status);
        emu::set_emulation_status(status);
        Ok(())
    }

    fn get_active_virtual_amiibo(&mut self, mut out_path: sf::OutMapAliasBuffer) -> Result<amiibo::VirtualAmiiboData> {
        log!("\n[emu] GetActiveVirtualAmiibo -- (...)\n");
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), resultsext::emu::ResultInvalidActiveVirtualAmiibo);

        let data = amiibo.produce_data()?;
        out_path.set_string(amiibo.path.clone());
        Ok(data)
    }

    fn set_active_virtual_amiibo(&mut self, path: sf::InMapAliasBuffer) -> Result<()> {
        let path_str = path.get_string();
        log!("\n[emu] SetActiveVirtualAmiibo -- path: '{}'\n", path_str);
        let amiibo = amiibo::try_load_virtual_amiibo(path_str)?;
        result_return_unless!(amiibo.is_valid(), resultsext::emu::ResultInvalidLoadedVirtualAmiibo);

        emu::set_active_virtual_amiibo(amiibo);
        Ok(())
    }

    fn reset_active_virtual_amiibo(&mut self) -> Result<()> {
        log!("\n[emu] ResetActiveVirtualAmiibo -- (...)\n");
        emu::set_active_virtual_amiibo(amiibo::VirtualAmiibo::empty());
        Ok(())
    }

    fn get_active_virtual_amiibo_status(&mut self) -> Result<emu::VirtualAmiiboStatus> {
        log!("\n[emu] GetActiveVirtualAmiiboStatus -- (...)\n");
        let status = emu::get_active_virtual_amiibo_status();
        Ok(status)
    }

    fn set_active_virtual_amiibo_status(&mut self, status: emu::VirtualAmiiboStatus) -> Result<()> {
        log!("\n[emu] SetActiveVirtualAmiiboStatus -- status: {:?}\n", status);
        emu::set_active_virtual_amiibo_status(status);
        Ok(())
    }

    fn is_application_id_intercepted(&mut self, application_id: u64) -> Result<bool> {
        log!("\n[emu] IsApplicationIdIntercepted -- app_id: {:#X}\n", application_id);
        Ok(emu::is_application_id_intercepted(application_id))
    }

    fn try_parse_virtual_amiibo(&mut self, path: sf::InMapAliasBuffer) -> Result<amiibo::VirtualAmiiboData> {
        let path_str = path.get_string();
        log!("\n[emu] TryParseVirtualAmiibo -- path: '{}'\n", path_str);
        let amiibo = amiibo::try_load_virtual_amiibo(path_str)?;
        result_return_unless!(amiibo.is_valid(), resultsext::emu::ResultInvalidLoadedVirtualAmiibo);

        let data = amiibo.produce_data()?;
        Ok(data)
    }

    fn get_active_virtual_amiibo_areas(&mut self, out_areas: sf::OutMapAliasBuffer) -> Result<u32> {
        log!("\n[emu] GetActiveVirtualAmiiboAreas -- (...)\n");
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), resultsext::emu::ResultInvalidActiveVirtualAmiibo);

        let areas: &mut [amiibo::VirtualAmiiboAreaEntry] = out_areas.get_mut_slice();

        let count = areas.len().min(amiibo.areas.areas.len());
        for i in 0..count {
            areas[i] = amiibo.areas.areas[i];
        }

        Ok(count as u32)
    }
    
    fn get_active_virtual_amiibo_current_area(&mut self) -> Result<nfp::AccessId> {
        log!("\n[emu] GetActiveVirtualAmiiboCurrentArea -- (...)\n");
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), resultsext::emu::ResultInvalidActiveVirtualAmiibo);

        match amiibo.get_current_area() {
            Some(area_entry) => Ok(area_entry.access_id),
            None => Err(resultsext::emu::ResultInvalidVirtualAmiiboAccessId::make())
        }
    }
    
    fn set_active_virtual_amiibo_current_area(&mut self, access_id: nfp::AccessId) -> Result<()> {
        log!("\n[emu] SetActiveVirtualAmiiboCurrentArea -- access_id: {:#X}\n", access_id);
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), resultsext::emu::ResultInvalidActiveVirtualAmiibo);

        if amiibo.set_current_area(access_id) {
            Ok(())
        }
        else {
            Err(resultsext::emu::ResultInvalidVirtualAmiiboAccessId::make())
        }
    }
}

impl server::IService for EmulationService {
    fn get_name() -> &'static str {
        nul!("emuiibo")
    }

    fn get_max_sesssions() -> i32 {
        20
    }
}