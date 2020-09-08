use nx::result::*;
use serde::{Serialize, Deserialize};
use alloc::string::String;
use alloc::vec::Vec;
use nx::service::fspsrv;
use nx::service::fspsrv::IFileSystem;
use nx::mem;
use nx::sync;
use nx::fs;
use nx::util;
use nx::ipc::sf::mii;
use nx::ipc::sf::nfp;

use crate::fsext;

#[derive(Copy, Clone)]
#[repr(C)]
pub struct VirtualAmiiboUuidInfo {
    use_random_uuid: bool,
    uuid: [u8; 10]
}

#[derive(Copy, Clone)]
#[repr(C)]
pub struct VirtualAmiiboData {
    uuid_info: VirtualAmiiboUuidInfo,
    name: util::CString<41>,
    first_write_date: nfp::Date,
    last_write_date: nfp::Date,
    mii_charinfo: mii::CharInfo
}

#[derive(Serialize, Deserialize, Debug)]
pub struct VirtualAmiiboId {
    pub game_character_id: u16,
    pub character_variant: u8,
    pub figure_type: u8,
    pub model_number: u16,
    pub series: u8
}

impl VirtualAmiiboId {
    pub const fn empty() -> Self {
        Self { character_variant: 0, figure_type: 0, game_character_id: 0, model_number: 0, series: 0 }
    }
}

#[derive(Serialize, Deserialize, Debug)]
pub struct VirtualAmiiboDate {
    pub y: u16,
    pub m: u8,
    pub d: u8
}

impl VirtualAmiiboDate {
    pub const fn empty() -> Self {
        Self { y: 0, m: 0, d: 0 }
    }

    pub const fn to_date(&self) -> nfp::Date {
        nfp::Date { year: self.y, month: self.m, day: self.d }
    }
}

#[derive(Serialize, Deserialize, Debug)]
pub struct VirtualAmiiboInfo {
    first_write_date: VirtualAmiiboDate,
    id: VirtualAmiiboId,
    last_write_date: VirtualAmiiboDate,
    mii_charinfo_file: String,
    name: String,
    uuid: Option<Vec<u8>>,
    version: u16,
    write_counter: u16
}

impl VirtualAmiiboInfo {
    pub const fn empty() -> Self {
        Self {
            first_write_date: VirtualAmiiboDate::empty(),
            id: VirtualAmiiboId::empty(),
            last_write_date: VirtualAmiiboDate::empty(),
            mii_charinfo_file: String::new(),
            name: String::new(),
            uuid: None,
            version: 0,
            write_counter: 0
        }
    }
}

pub struct VirtualAmiibo {
    pub info: VirtualAmiiboInfo,
    pub mii_charinfo: mii::CharInfo,
    pub path: String
}

impl VirtualAmiibo {
    pub const fn empty() -> Self {
        Self { info: VirtualAmiiboInfo::empty(), mii_charinfo: mii::CharInfo { data: [0; 0x58] }, path: String::new() }
    }

    pub fn new(info: VirtualAmiiboInfo, path: String) -> Result<Self> {
        let mut amiibo = Self { info: info, mii_charinfo: mii::CharInfo { data: [0; 0x58] }, path: path };
        amiibo.mii_charinfo = amiibo.load_mii_charinfo()?;
        Ok(amiibo)
    }

    pub fn is_valid(&self) -> bool {
        !self.path.is_empty()
    }

    pub fn load_mii_charinfo(&self) -> Result<mii::CharInfo> {
        let mut mii_charinfo: mii::CharInfo = unsafe { core::mem::zeroed() };
        let mut mii_charinfo_file = fs::open_file(format!("{}/{}", self.path, self.info.mii_charinfo_file), fs::FileOpenOption::Read())?;
        let mii_charinfo_size = mii_charinfo_file.get_size()?;
        result_return_unless!(mii_charinfo_size == core::mem::size_of::<mii::CharInfo>(), 0xBADD);
        mii_charinfo_file.read(&mut mii_charinfo, mii_charinfo_size)?;
        Ok(mii_charinfo)
    }

    pub fn produce_data(&self) -> Result<VirtualAmiiboData> {
        let mut data: VirtualAmiiboData = unsafe { core::mem::zeroed() };
        
        match self.info.uuid.as_ref() {
            Some(uuid) => {
                for i in 0..10 {
                    data.uuid_info.uuid[i] = uuid[i];
                }
            },
            None => data.uuid_info.use_random_uuid = true
        };
        data.name.set_string(self.info.name.clone())?;
        data.first_write_date = self.info.first_write_date.to_date();
        data.last_write_date = self.info.last_write_date.to_date();
        data.mii_charinfo = self.mii_charinfo;

        Ok(data)
    }

    pub fn produce_tag_info(&self) -> nfp::TagInfo {
        let mut tag_info: nfp::TagInfo = unsafe { core::mem::zeroed() };
        tag_info.uuid_length = tag_info.uuid.len() as u8;
        unsafe {
            match self.info.uuid.as_ref() {
                Some(uuid) => core::ptr::copy(uuid.as_ptr(), tag_info.uuid.as_mut_ptr(), tag_info.uuid.len()),
                None => {
                    // TODO: random
                    core::ptr::write_bytes(tag_info.uuid.as_mut_ptr(), 0xBE, tag_info.uuid.len());
                }
            };
        }
        tag_info.tag_type = u32::max_value();
        tag_info.protocol = u32::max_value();
        tag_info
    }

    pub fn produce_register_info(&self) -> nfp::RegisterInfo {
        let mut register_info: nfp::RegisterInfo = unsafe { core::mem::zeroed() };
        register_info.mii_charinfo = self.mii_charinfo;
        let _ = register_info.name.set_string(self.info.name.clone());
        register_info.first_write_date = self.info.first_write_date.to_date();
        register_info
    }

    pub fn produce_common_info(&self) -> nfp::CommonInfo {
        let mut common_info: nfp::CommonInfo = unsafe { core::mem::zeroed() };
        common_info.last_write_date = self.info.first_write_date.to_date();
        common_info.application_area_size = 0xD8;
        common_info.version = self.info.version;
        common_info.write_counter = self.info.write_counter;
        common_info
    }

    pub fn produce_model_info(&self) -> nfp::ModelInfo {
        let mut model_info: nfp::ModelInfo = unsafe { core::mem::zeroed() };
        model_info.game_character_id = self.info.id.game_character_id;
        model_info.character_variant = self.info.id.character_variant;
        model_info.figure_type = self.info.id.figure_type;
        model_info.model_number = self.info.id.model_number;
        model_info.series = self.info.id.series;
        model_info
    }
}

pub fn try_load_virtual_amiibo(path: String) -> Result<VirtualAmiibo> {
    let amiibo_flag_file = format!("{}/amiibo.flag", path);
    result_return_unless!(fsext::exists_file(amiibo_flag_file), 0xBEBE);

    let amiibo_json_file = format!("{}/amiibo.json", path);
    result_return_unless!(fsext::exists_file(amiibo_json_file.clone()), 0xBEBE);

    let mut amiibo_json = fs::open_file(amiibo_json_file, fs::FileOpenOption::Read())?;
    let mut amiibo_json_data: Vec<u8> = vec![0; amiibo_json.get_size()?];
    amiibo_json.read(amiibo_json_data.as_mut_ptr(), amiibo_json_data.len())?;
    if let Ok(amiibo_json_str) = core::str::from_utf8(amiibo_json_data.as_slice()) {
        if let Ok(virtual_amiibo_info) = serde_json::from_str::<VirtualAmiiboInfo>(amiibo_json_str) {
            return VirtualAmiibo::new(virtual_amiibo_info, path.clone());
        }
    }
    Err(ResultCode::new(0xBEBE))
}