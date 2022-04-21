use nx::{result::*, service::mii, fs};
use alloc::{string::String, vec::Vec};
use serde::{Serialize, Deserialize};
use crate::{fsext, miiext};
use super::{compat, fmt};

#[derive(Serialize, Deserialize, Clone, Debug)]
#[allow(non_snake_case)]
pub struct VirtualAmiiboCommonInfo {
    pub writeCounter: u16,
    pub lastWriteDate: String,
    pub version: u16
}

#[derive(Serialize, Deserialize, Clone, Debug)]
#[allow(non_snake_case)]
pub struct VirtualAmiiboTagInfo {
    pub uuid: Option<String>,
    pub randomUuid: bool
}

#[derive(Serialize, Deserialize, Clone, Debug)]
#[allow(non_snake_case)]
pub struct VirtualAmiiboModelInfo {
    pub amiiboId: String
}

#[derive(Serialize, Deserialize, Clone, Debug)]
#[allow(non_snake_case)]
pub struct VirtualAmiiboRegisterInfo {
    pub name: String,
    pub miiCharInfo: String,
    pub firstWriteDate: String
}

fn convert_date(info_date: &String) -> fmt::VirtualAmiiboDate {
    // Dates are in "2020-12-12"-like format

    let y_str = &info_date[0..4];
    let y = u16::from_str_radix(y_str, 10).unwrap();

    let m_str = &info_date[5..7];
    let m = u8::from_str_radix(m_str, 10).unwrap();

    let d_str = &info_date[8..10];
    let d = u8::from_str_radix(d_str, 10).unwrap();

    fmt::VirtualAmiiboDate { y, m, d }
}

// Why did I use this format back then... it's quite annoying to parse

#[inline]
fn convert_from_hex(hex: String) -> Vec<u8> {
    (0..hex.len()).step_by(2).map(|i| u8::from_str_radix(&hex[i..i + 2], 16)).collect::<core::result::Result<Vec<u8>, _>>().unwrap()
}

#[inline]
fn get_raw_hex_bytes<const S: usize>(hex: String) -> [u8; S] {
    let mut raw: [u8; S] = [0; S];
    let bytes = convert_from_hex(hex);

    for i in 0..S {
        raw[i] = bytes[i];
    }

    raw
}

fn convert_amiibo_id(info_id: String) -> fmt::VirtualAmiiboId {
    let raw_id = get_raw_hex_bytes::<8>(info_id);
    unsafe {
        core::mem::transmute(raw_id)
    }
}

fn convert_uuid(info_uuid: String) -> Vec<u8> {
    convert_from_hex(info_uuid)
}

#[derive(Clone, Debug)]
pub struct VirtualAmiibo {
    path: String,
    common_info: VirtualAmiiboCommonInfo,
    tag_info: VirtualAmiiboTagInfo,
    model_info: VirtualAmiiboModelInfo,
    register_info: VirtualAmiiboRegisterInfo,
    mii_charinfo: mii::CharInfo
}

impl super::VirtualAmiiboFormat for VirtualAmiibo {
    fn try_load(path: String) -> Result<Self> {
        let common_json_path = format!("{}/common.json", path);
        let common_info = read_deserialize_json!(common_json_path => VirtualAmiiboCommonInfo)?;

        let tag_json_path = format!("{}/tag.json", path);
        let tag_info = read_deserialize_json!(tag_json_path => VirtualAmiiboTagInfo)?;

        let model_json_path = format!("{}/model.json", path);
        let model_info = read_deserialize_json!(model_json_path => VirtualAmiiboModelInfo)?;

        let register_json_path = format!("{}/register.json", path);
        let register_info = read_deserialize_json!(register_json_path => VirtualAmiiboRegisterInfo)?;

        let mii_charinfo_path = format!("{}/{}", path, register_info.miiCharInfo);
        // If newly generated, charinfo may not exist yet
        let mii_charinfo = match fs::get_entry_type(mii_charinfo_path.clone()).is_err() {
            true => {
                let mii_charinfo = miiext::generate_random_mii()?;
                let mut file = fs::open_file(mii_charinfo_path, fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
                file.write_val(mii_charinfo)?;
                mii_charinfo
            },
            false => {
                let mut file = fs::open_file(mii_charinfo_path, fs::FileOpenOption::Read())?;
                file.read_val()?
            }
        };

        Ok(Self { path, common_info, tag_info, model_info, register_info, mii_charinfo })
    }
}

impl compat::DeprecatedVirtualAmiiboFormat for VirtualAmiibo {
    fn convert(&self) -> Result<fmt::VirtualAmiibo> {
        // Save converted amiibo
        let name = fsext::get_path_file_name(self.path.clone());
        let path = Self::find_convert_path(self.path.clone(), name);
        let _ = fs::create_directory(path.clone());

        let mut amiibo = fmt::VirtualAmiibo {
            info: fmt::VirtualAmiiboInfo {
                first_write_date: convert_date(&self.register_info.firstWriteDate),
                id: convert_amiibo_id(self.model_info.amiiboId.clone()),
                last_write_date: convert_date(&self.common_info.lastWriteDate),
                mii_charinfo_file: String::from("mii-charinfo.bin"),
                name: self.register_info.name.clone(),
                uuid: self.tag_info.uuid.as_ref().map(|uuid| convert_uuid(uuid.clone())),
                version: self.common_info.version as u8,
                write_counter: self.common_info.writeCounter
            },
            mii_charinfo: self.mii_charinfo,
            areas: fmt::VirtualAmiiboAreaInfo::empty(),
            path: path.clone()
        };

        let existing_access_id = fmt::generate_areas_json(path.clone())?;
        if let Some(existing_id) = existing_access_id {
            amiibo.ensure_area_registered(existing_id, fmt::DEFAULY_EMPTY_AREA_PROGRAM_ID);
        }

        amiibo.save()?;

        // Save deprecated amiibo inside /v3 dir
        let deprecated_path = format!("{}/v3", path);
        fs::create_directory(deprecated_path.clone())?;

        let old_common_json_path = format!("{}/common.json", self.path);
        let new_common_json_path = format!("{}/common.json", deprecated_path);
        fs::rename_file(old_common_json_path, new_common_json_path)?;

        let old_tag_json_path = format!("{}/tag.json", self.path);
        let new_tag_json_path = format!("{}/tag.json", deprecated_path);
        fs::rename_file(old_tag_json_path, new_tag_json_path)?;

        let old_model_json_path = format!("{}/model.json", self.path);
        let new_model_json_path = format!("{}/model.json", deprecated_path);
        fs::rename_file(old_model_json_path, new_model_json_path)?;

        let old_register_json_path = format!("{}/register.json", self.path);
        let new_register_json_path = format!("{}/register.json", deprecated_path);
        fs::rename_file(old_register_json_path, new_register_json_path)?;

        let old_mii_charinfo_path = format!("{}/{}", self.path, self.register_info.miiCharInfo);
        let new_mii_charinfo_path = format!("{}/{}", deprecated_path, self.register_info.miiCharInfo);
        fs::rename_file(old_mii_charinfo_path, new_mii_charinfo_path)?;

        if self.path != path {
            fs::delete_directory_recursively(self.path.clone())?;
        }

        Ok(amiibo)
    }
}