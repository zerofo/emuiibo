use nx::ipc::sf::sm;
use nx::result::*;
use nx::ipc::sf;
use nx::ipc::server;
use nx::ipc::sf::nfp;
use nx::version;
use alloc::string::String;

use crate::resultsext;
use crate::emu;
use crate::amiibo;
use crate::fsext;

ipc_sf_define_interface_trait! {
    trait IEmulationService {
        get_version [0, version::VersionInterval::all()]: () => (version: emu::Version);
        get_virtual_amiibo_directory [1, version::VersionInterval::all()]: (out_path: sf::OutMapAliasBuffer<u8>) => ();
        get_emulation_status [2, version::VersionInterval::all()]: () => (status: emu::EmulationStatus);
        set_emulation_status [3, version::VersionInterval::all()]: (status: emu::EmulationStatus) => ();
        get_active_virtual_amiibo [4, version::VersionInterval::all()]: (out_path: sf::OutMapAliasBuffer<u8>) => (virtual_amiibo: amiibo::VirtualAmiiboData);
        set_active_virtual_amiibo [5, version::VersionInterval::all()]: (path: sf::InMapAliasBuffer<u8>) => ();
        reset_active_virtual_amiibo [6, version::VersionInterval::all()]: () => ();
        get_active_virtual_amiibo_status [7, version::VersionInterval::all()]: () => (status: emu::VirtualAmiiboStatus);
        set_active_virtual_amiibo_status [8, version::VersionInterval::all()]: (status: emu::VirtualAmiiboStatus) => ();
        is_application_id_intercepted [9, version::VersionInterval::all()]: (application_id: u64) => (is_intercepted: bool);
        try_parse_virtual_amiibo [10, version::VersionInterval::all()]: (path: sf::InMapAliasBuffer<u8>) => (virtual_amiibo: amiibo::VirtualAmiiboData);
        get_active_virtual_amiibo_areas [11, version::VersionInterval::all()]: (out_areas: sf::OutMapAliasBuffer<amiibo::VirtualAmiiboAreaEntry>) => (count: u32);
        get_active_virtual_amiibo_current_area [12, version::VersionInterval::all()]: () => (access_id: nfp::AccessId);
        set_active_virtual_amiibo_current_area [13, version::VersionInterval::all()]: (access_id: nfp::AccessId) => ();
    }
}

pub struct EmulationService {
    session: sf::Session
}

impl sf::IObject for EmulationService {
    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.session
    }

    ipc_sf_object_impl_default_command_metadata!();
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

    fn get_virtual_amiibo_directory(&mut self, mut out_path: sf::OutMapAliasBuffer<u8>) -> Result<()> {
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

    fn get_active_virtual_amiibo(&mut self, mut out_path: sf::OutMapAliasBuffer<u8>) -> Result<amiibo::VirtualAmiiboData> {
        log!("\n[emu] GetActiveVirtualAmiibo -- (...)\n");
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), resultsext::emu::ResultInvalidActiveVirtualAmiibo);

        let data = amiibo.produce_data()?;
        out_path.set_string(amiibo.path.clone());
        Ok(data)
    }

    fn set_active_virtual_amiibo(&mut self, path: sf::InMapAliasBuffer<u8>) -> Result<()> {
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

    fn try_parse_virtual_amiibo(&mut self, path: sf::InMapAliasBuffer<u8>) -> Result<amiibo::VirtualAmiiboData> {
        let path_str = path.get_string();
        log!("\n[emu] TryParseVirtualAmiibo -- path: '{}'\n", path_str);
        let amiibo = amiibo::try_load_virtual_amiibo(path_str)?;
        result_return_unless!(amiibo.is_valid(), resultsext::emu::ResultInvalidLoadedVirtualAmiibo);

        let data = amiibo.produce_data()?;
        Ok(data)
    }

    fn get_active_virtual_amiibo_areas(&mut self, out_areas: sf::OutMapAliasBuffer<amiibo::VirtualAmiiboAreaEntry>) -> Result<u32> {
        log!("\n[emu] GetActiveVirtualAmiiboAreas -- (...)\n");
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), resultsext::emu::ResultInvalidActiveVirtualAmiibo);

        let areas = out_areas.get_mut_slice();
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
    fn get_name() -> sm::ServiceName {
        sm::ServiceName::new("emuiibo")
    }

    fn get_max_sesssions() -> i32 {
        20
    }
}