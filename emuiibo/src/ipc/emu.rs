use nx::ipc::sf::sm;
use nx::result::*;
use nx::ipc::sf;
use nx::ipc::server;
use nx::ipc::sf::nfp;
use nx::version;
use alloc::string::String;
use crate::rc;
use crate::emu;
use crate::amiibo;
use crate::amiibo::VirtualAmiiboFormat;

ipc_sf_define_interface_trait! {
    trait IEmulationService {
        get_version [0, version::VersionInterval::all()]: () => (version: emu::Version);
        get_virtual_amiibo_directory [1, version::VersionInterval::all()]: (out_path: sf::OutMapAliasBuffer<u8>) => ();
        get_emulation_status [2, version::VersionInterval::all()]: () => (status: emu::EmulationStatus);
        set_emulation_status [3, version::VersionInterval::all()]: (status: emu::EmulationStatus) => ();
        get_active_virtual_amiibo [4, version::VersionInterval::all()]: (out_path: sf::OutMapAliasBuffer<u8>) => (virtual_amiibo: amiibo::fmt::VirtualAmiiboData);
        set_active_virtual_amiibo [5, version::VersionInterval::all()]: (path: sf::InMapAliasBuffer<u8>) => ();
        reset_active_virtual_amiibo [6, version::VersionInterval::all()]: () => ();
        get_active_virtual_amiibo_status [7, version::VersionInterval::all()]: () => (status: emu::VirtualAmiiboStatus);
        set_active_virtual_amiibo_status [8, version::VersionInterval::all()]: (status: emu::VirtualAmiiboStatus) => ();
        is_application_id_intercepted [9, version::VersionInterval::all()]: (application_id: u64) => (is_intercepted: bool);
        try_parse_virtual_amiibo [10, version::VersionInterval::all()]: (path: sf::InMapAliasBuffer<u8>) => (virtual_amiibo: amiibo::fmt::VirtualAmiiboData);
        get_active_virtual_amiibo_areas [11, version::VersionInterval::all()]: (out_areas: sf::OutMapAliasBuffer<amiibo::fmt::VirtualAmiiboAreaEntry>) => (count: u32);
        get_active_virtual_amiibo_current_area [12, version::VersionInterval::all()]: () => (access_id: nfp::AccessId);
        set_active_virtual_amiibo_current_area [13, version::VersionInterval::all()]: (access_id: nfp::AccessId) => ();
    }
}

pub struct EmulationService {
    dummy_session: sf::Session
}

impl sf::IObject for EmulationService {
    ipc_sf_object_impl_default_command_metadata!();

    fn get_session(&mut self) -> &mut sf::Session {
        &mut self.dummy_session
    }
}

impl IEmulationService for EmulationService {
    fn get_version(&mut self) -> Result<emu::Version> {
        log!("GetVersion -- (...)\n");
        Ok(emu::CURRENT_VERSION)
    }

    fn get_virtual_amiibo_directory(&mut self, mut out_path: sf::OutMapAliasBuffer<u8>) -> Result<()> {
        log!("GetVirtualAmiiboDirectory -- (...)\n");
        out_path.set_string(String::from(amiibo::VIRTUAL_AMIIBO_DIR));
        Ok(())
    }

    fn get_emulation_status(&mut self) -> Result<emu::EmulationStatus> {
        log!("GetEmulationStatus -- (...)\n");
        let status = emu::get_emulation_status();
        Ok(status)
    }

    fn set_emulation_status(&mut self, status: emu::EmulationStatus) -> Result<()> {
        log!("SetEmulationStatus -- status: {:?}\n", status);
        emu::set_emulation_status(status);
        Ok(())
    }

    fn get_active_virtual_amiibo(&mut self, mut out_path: sf::OutMapAliasBuffer<u8>) -> Result<amiibo::fmt::VirtualAmiiboData> {
        log!("GetActiveVirtualAmiibo -- (...)\n");
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), rc::ResultInvalidActiveVirtualAmiibo);

        let data = amiibo.produce_data()?;
        out_path.set_string(amiibo.path.clone());
        Ok(data)
    }

    fn set_active_virtual_amiibo(&mut self, path: sf::InMapAliasBuffer<u8>) -> Result<()> {
        let path_str = path.get_string();
        log!("SetActiveVirtualAmiibo -- path: '{}'\n", path_str);
        let amiibo = amiibo::fmt::VirtualAmiibo::try_load(path_str)?;
        result_return_unless!(amiibo.is_valid(), rc::ResultInvalidLoadedVirtualAmiibo);

        emu::set_active_virtual_amiibo(amiibo);
        Ok(())
    }

    fn reset_active_virtual_amiibo(&mut self) -> Result<()> {
        log!("ResetActiveVirtualAmiibo -- (...)\n");
        emu::set_active_virtual_amiibo(amiibo::fmt::VirtualAmiibo::empty());
        Ok(())
    }

    fn get_active_virtual_amiibo_status(&mut self) -> Result<emu::VirtualAmiiboStatus> {
        log!("GetActiveVirtualAmiiboStatus -- (...)\n");
        let status = emu::get_active_virtual_amiibo_status();
        Ok(status)
    }

    fn set_active_virtual_amiibo_status(&mut self, status: emu::VirtualAmiiboStatus) -> Result<()> {
        log!("SetActiveVirtualAmiiboStatus -- status: {:?}\n", status);
        emu::set_active_virtual_amiibo_status(status);
        Ok(())
    }

    fn is_application_id_intercepted(&mut self, application_id: u64) -> Result<bool> {
        log!("IsApplicationIdIntercepted -- app_id: {:#X}\n", application_id);
        Ok(emu::is_application_id_intercepted(application_id))
    }

    fn try_parse_virtual_amiibo(&mut self, path: sf::InMapAliasBuffer<u8>) -> Result<amiibo::fmt::VirtualAmiiboData> {
        let path_str = path.get_string();
        log!("TryParseVirtualAmiibo -- path: '{}'\n", path_str);
        let amiibo = amiibo::fmt::VirtualAmiibo::try_load(path_str)?;
        result_return_unless!(amiibo.is_valid(), rc::ResultInvalidLoadedVirtualAmiibo);

        let data = amiibo.produce_data()?;
        Ok(data)
    }

    fn get_active_virtual_amiibo_areas(&mut self, out_areas: sf::OutMapAliasBuffer<amiibo::fmt::VirtualAmiiboAreaEntry>) -> Result<u32> {
        log!("GetActiveVirtualAmiiboAreas -- (...)\n");
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), rc::ResultInvalidActiveVirtualAmiibo);

        let areas = out_areas.get_mut_slice();
        let count = areas.len().min(amiibo.areas.areas.len());
        for i in 0..count {
            areas[i] = amiibo.areas.areas[i];
        }

        Ok(count as u32)
    }
    
    fn get_active_virtual_amiibo_current_area(&mut self) -> Result<nfp::AccessId> {
        log!("GetActiveVirtualAmiiboCurrentArea -- (...)\n");
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), rc::ResultInvalidActiveVirtualAmiibo);

        match amiibo.get_current_area() {
            Some(area_entry) => Ok(area_entry.access_id),
            None => Err(rc::ResultInvalidVirtualAmiiboAccessId::make())
        }
    }
    
    fn set_active_virtual_amiibo_current_area(&mut self, access_id: nfp::AccessId) -> Result<()> {
        log!("SetActiveVirtualAmiiboCurrentArea -- access_id: {:#X}\n", access_id);
        let amiibo = emu::get_active_virtual_amiibo();
        result_return_unless!(amiibo.is_valid(), rc::ResultInvalidActiveVirtualAmiibo);

        if amiibo.set_current_area(access_id) {
            Ok(())
        }
        else {
            Err(rc::ResultInvalidVirtualAmiiboAccessId::make())
        }
    }
}

impl server::ISessionObject for EmulationService {}

impl server::IServerObject for EmulationService {
    fn new() -> Self {
        Self {
            dummy_session: sf::Session::new()
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