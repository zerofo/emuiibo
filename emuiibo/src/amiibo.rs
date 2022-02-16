use nx::result::*;
use serde::{Serialize, Deserialize};
use alloc::string::String;
use alloc::vec::Vec;
use nx::fs;
use nx::util;
use nx::rand;
use nx::rand::RandomGenerator;
use nx::ipc::sf::mii;
use nx::ipc::sf::nfp;

use crate::fsext;
use crate::miiext;

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct VirtualAmiiboUuidInfo {
    use_random_uuid: bool,
    uuid: [u8; 10]
}

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct VirtualAmiiboData {
    uuid_info: VirtualAmiiboUuidInfo,
    name: util::CString<41>,
    first_write_date: nfp::Date,
    last_write_date: nfp::Date,
    mii_charinfo: mii::CharInfo
}

// Note: actual amiibo ID in amiibos (nfp services have a different ID type)

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

    pub const fn from_date(date: nfp::Date) -> Self {
        Self { y: date.year, m: date.month, d: date.day }
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
    version: u8,
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

const DEFAULT_MII_NAME: &'static str = "emuiibo";

pub struct VirtualAmiibo {
    pub info: VirtualAmiiboInfo,
    pub mii_charinfo: mii::CharInfo,
    pub path: String
}

impl VirtualAmiibo {
    pub const fn empty() -> Self {
        // Can't use Default with charinfo here as the function MUST be const - waiting for const traits...
        Self {
            info: VirtualAmiiboInfo::empty(),
            mii_charinfo: unsafe { core::mem::MaybeUninit::zeroed().assume_init() },
            path: String::new()
        }
    }

    pub fn new(info: VirtualAmiiboInfo, path: String) -> Result<Self> {
        let mut amiibo = Self {
            info: info,
            mii_charinfo: Default::default(),
            path: path
        };
        amiibo.mii_charinfo = amiibo.load_mii_charinfo()?;
        Ok(amiibo)
    }

    pub fn is_valid(&self) -> bool {
        !self.path.is_empty()
    }

    pub fn load_mii_charinfo(&self) -> Result<mii::CharInfo> {
        let mii_charinfo_path = format!("{}/{}", self.path, self.info.mii_charinfo_file);
        if fsext::exists_file(mii_charinfo_path.clone()) {
            let mut mii_charinfo_file = fs::open_file(mii_charinfo_path, fs::FileOpenOption::Read())?;
            Ok(mii_charinfo_file.read_val()?)
        }
        else {
            let mut random_mii = miiext::generate_random_mii()?;
            random_mii.name.set_str(DEFAULT_MII_NAME)?;
            
            let mut mii_charinfo_file = fs::open_file(mii_charinfo_path, fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
            mii_charinfo_file.write_val(random_mii)?;
            Ok(random_mii)
        }
    }

    pub fn produce_data(&self) -> Result<VirtualAmiiboData> {
        let mut data: VirtualAmiiboData = Default::default();
        match self.info.uuid.as_ref() {
            Some(uuid) => {
                for i in 0..data.uuid_info.uuid.len() {
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

    pub fn produce_tag_info(&self) -> Result<nfp::TagInfo> {
        let mut tag_info = nfp::TagInfo {
            uuid: [0; 0xA],
            uuid_length: 0,
            reserved_1: [0; 0x15],
            protocol: u32::MAX,
            tag_type: u32::MAX,
            reserved_2: [0; 0x30]
        };
        unsafe {
            match self.info.uuid.as_ref() {
                Some(uuid) => {
                    let uuid_len = uuid.len().min(tag_info.uuid.len());
                    tag_info.uuid_length = uuid_len as u8;
                    core::ptr::copy(uuid.as_ptr(), tag_info.uuid.as_mut_ptr(), uuid_len);
                },
                None => {
                    let mut rng = rand::SplCsrngGenerator::new()?;
                    rng.random_bytes(tag_info.uuid.as_mut_ptr(), tag_info.uuid.len())?;
                }
            };
        }
        Ok(tag_info)
    }

    pub fn produce_register_info(&self) -> Result<nfp::RegisterInfo> {
        Ok(nfp::RegisterInfo {
            mii_charinfo: self.mii_charinfo,
            first_write_date: self.info.first_write_date.to_date(),
            name: util::CString::from_string(self.info.name.clone())?,
            unk: 0,
            reserved: [0; 0x7A]
        })
    }

    pub fn produce_common_info(&self) -> Result<nfp::CommonInfo> {
        Ok(nfp::CommonInfo {
            last_write_date: self.info.last_write_date.to_date(),
            write_counter: self.info.write_counter,
            version: self.info.version,
            pad: 0,
            application_area_size: 0xD8,
            reserved: [0; 0x34]
        })
    }

    pub fn produce_model_info(&self) -> Result<nfp::ModelInfo> {
        Ok(nfp::ModelInfo {
            game_character_id: self.info.id.game_character_id,
            character_variant: self.info.id.character_variant,
            series: self.info.id.series,
            model_number: self.info.id.model_number, // Note: we should technically reverse it since nfp wants it reversed... but it only works this way
            figure_type: self.info.id.figure_type,
            reserved: [0; 0x39]
        })
    }

    pub fn produce_register_info_private(&self) -> Result<nfp::RegisterInfoPrivate> {
        Ok(nfp::RegisterInfoPrivate {
            mii_store_data: mii::StoreData::from_charinfo(self.mii_charinfo)?,
            first_write_date: self.info.first_write_date.to_date(),
            name: util::CString::from_string(self.info.name.clone())?,
            unk: 0,
            reserved: [0; 0x8E]
        })
    }

    pub fn produce_admin_info(&self) -> Result<nfp::AdminInfo> {
        // TODO
        Ok(nfp::AdminInfo {
            program_id: 0x01006A800016E000,
            access_id: 0x34F80200,
            crc32_change_counter: 10,
            flags: nfp::AdminInfoFlags::IsInitialized() | nfp::AdminInfoFlags::HasApplicationArea(),
            unk_0x2: 0x2,
            console_type: nfp::ProgramIdConsoleType::NintendoSwitch,
            pad: [0; 0x7],
            reserved: [0; 0x28]
        })
    }

    pub fn update_from_register_info_private(&mut self, register_info_private: &nfp::RegisterInfoPrivate) -> Result<()> {
        self.mii_charinfo = register_info_private.mii_store_data.to_charinfo()?;
        self.info.first_write_date = VirtualAmiiboDate::from_date(register_info_private.first_write_date);
        self.info.name = register_info_private.name.get_string()?;

        self.notify_written()
    }

    pub fn save(&self) -> Result<()> {
        if let Ok(data) = serde_json::to_vec_pretty(&self.info) {
            let amiibo_json_file = format!("{}/amiibo.json", self.path);
            let _ = fs::delete_file(amiibo_json_file.clone());
            let mut amiibo_json = fs::open_file(amiibo_json_file.clone(), fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
            amiibo_json.write(data.as_ptr(), data.len())?;

            let mii_charinfo_path = format!("{}/{}", self.path, self.info.mii_charinfo_file);
            let _ = fs::delete_file(mii_charinfo_path.clone());
            let mut mii_charinfo_file = fs::open_file(mii_charinfo_path.clone(), fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
            mii_charinfo_file.write_val(self.mii_charinfo)?;

            Ok(())
        }
        else {
            Err(ResultCode::new(0xBEBE))
        }
    }

    pub fn notify_written(&mut self) -> Result<()> {
        if self.info.write_counter < 0xFFFF {
            self.info.write_counter += 1;
        }
        self.save()
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