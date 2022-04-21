use crate::area;
use crate::fsext;

use super::{bin, compat, fmt};
use nx::fs;
use nx::result::*;
use alloc::string::String;

// Format used in emuiibo v0.1
// It's not an actual virtual amiibo format, raw bin dumps were directly read/used

#[derive(Debug)]
pub struct VirtualAmiibo {
    raw_bin_path: String,
    raw_bin: bin::RawFormat
}

impl super::VirtualAmiiboFormat for VirtualAmiibo {
    fn try_load(path: String) -> Result<Self> {
        let mut file = fs::open_file(path.clone(), fs::FileOpenOption::Read())?;
        let raw_bin: bin::RawFormat = file.read_val()?;
        Ok(Self { raw_bin_path: path, raw_bin })
    }
}

const RETAIL_KEY_SET_FILE: &str = "sdmc:/switch/key_retail.bin";

impl compat::DeprecatedVirtualAmiiboFormat for VirtualAmiibo {
    fn convert(&self) -> Result<fmt::VirtualAmiibo> {
        /* TODO: uncomment when amiibo crypto stuff is implemented
        let conv_bin = bin::ConvertedFormat::from_raw(&self.raw_bin);

        if let Ok(key_set_file) = fs::open_file(String::from(RETAIL_KEY_SET_FILE), fs::FileOpenOption::Read()) {
            if let Ok(key_set) = key_set_file.read_val::<bin::RetailKeySet>() {
                let derived_key_set = bin::DerivedKeySet::derive_from(&key_set);

                let plain_bin = bin::PlainFormat::decrypt_from_converted(&conv_bin, &derived_key_set);
                return Some(Self::convert_from_plain(&plain_bin));
            }
        }
        */

        // Convert raw format
        let conv_bin = bin::ConvertedFormat::from_raw(&self.raw_bin);
        let plain_bin = bin::PlainFormat::from_converted(&conv_bin);

        // Save converted amiibo
        let name = fsext::get_path_file_name_without_extension(self.raw_bin_path.clone());
        let path = Self::find_convert_path(String::new(), name);
        fs::create_directory(path.clone())?;

        let mii_charinfo_name = String::from("mii-charinfo.bin");
        let mut amiibo = plain_bin.to_virtual_amiibo(path.clone(), mii_charinfo_name)?;

        // Save application area if present
        if plain_bin.dec_data.settings.flags.contains(bin::Flags::ApplicationAreaUsed()) {
            let access_id = plain_bin.dec_data.settings.access_id_be.swap_bytes();
            let program_id = plain_bin.dec_data.settings.program_id_be.swap_bytes();
            let bin_area = area::ApplicationArea::from(&amiibo, access_id);
            bin_area.create(plain_bin.dec_data.app_area.as_ptr(), plain_bin.dec_data.app_area.len(), false)?;

            amiibo.ensure_area_registered(access_id, program_id);
        }

        amiibo.save()?;

        // Save deprecated amiibo inside /v1 dir
        let deprecated_path = format!("{}/v1", path);
        fs::create_directory(deprecated_path.clone())?;

        let new_raw_bin_path = format!("{}/raw-format.bin", deprecated_path);
        fs::rename_file(self.raw_bin_path.clone(), new_raw_bin_path)?;

        {
            let conv_bin_path = format!("{}/converted-format.bin", deprecated_path);
            let mut conv_bin_file = fs::open_file(conv_bin_path, fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
            conv_bin_file.write_val(conv_bin)?;
        }
        {
            let plain_bin_path = format!("{}/plain-format.bin", deprecated_path);
            let mut plain_bin_file = fs::open_file(plain_bin_path, fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
            plain_bin_file.write_val(plain_bin)?;
        }

        Ok(amiibo)
    }
}