use nx::{result::*, fs};
use crate::amiibo::bin;

use super::{v1, v2, v3, fmt, VirtualAmiiboFormat};
use alloc::string::String;

const RETAIL_KEY_SET_FILE: &str = "sdmc:/switch/key_retail.bin";

pub trait DeprecatedVirtualAmiiboFormat: super::VirtualAmiiboFormat {
    fn convert(&self, key_set: Option<bin::RetailKeySet>) -> Result<fmt::VirtualAmiibo>;

    fn find_convert_path(old_path: String, name: String) -> String {
        let mut path = format!("{}/{}", super::VIRTUAL_AMIIBO_DIR, name);

        if path == old_path {
            return path;
        }

        let mut name_idx: usize = 0;
        loop {
            // Path already exists
            if fs::get_entry_type(path.clone()).is_ok() {
                name_idx += 1;
                path = format!("{}/{}_{}", super::VIRTUAL_AMIIBO_DIR, name, name_idx);
            }
            else {
                break;
            }
        }

        path
    }
}

fn convert_deprecated_virtual_amiibos_in_dir(path: String) -> Result<()> {
    let mut key_set: Option<bin::RetailKeySet> = None;
    if let Ok(mut key_set_file) = fs::open_file(String::from(RETAIL_KEY_SET_FILE), fs::FileOpenOption::Read()) {
        if let Ok(key_set_v) = key_set_file.read_val::<bin::RetailKeySet>() {
            log!("Found key_retail.bin --- old amiibo / raw dump conversions will include encrypted sections too!");
            key_set = Some(key_set_v);
        }
    }

    let mut dir = fs::open_directory(path.clone(), fs::DirectoryOpenMode::ReadDirectories() | fs::DirectoryOpenMode::ReadFiles())?;

    loop {
        if let Some(entry) = dir.read_next()? {
            let entry_path = format!("{}/{:?}", path, entry.name);
            log!("Analyzing entry '{}'...\n", entry_path);

            let maybe_new_amiibo = {
                if let Ok(v1_amiibo) = v1::VirtualAmiibo::try_load(entry_path.clone()) {
                    log!("Loaded v1 amiibo {:?} - converting it...\n", v1_amiibo);
                    Some(v1_amiibo.convert(key_set))
                }
                else if let Ok(v2_amiibo) = v2::VirtualAmiibo::try_load(entry_path.clone()) {
                    log!("Loaded v2 amiibo {:?} - converting it...\n", v2_amiibo);
                    Some(v2_amiibo.convert(key_set))
                }
                else if let Ok(v3_amiibo) = v3::VirtualAmiibo::try_load(entry_path.clone()) {
                    log!("Loaded v3 amiibo {:?} - converting it...\n", v3_amiibo);
                    Some(v3_amiibo.convert(key_set))
                }
                else {
                    None
                }
            };
            
            if let Some(new_amiibo_rc) = maybe_new_amiibo {
                match new_amiibo_rc {
                    Ok(new_amiibo) => log!("Converted new amiibo: {:?}\n", new_amiibo),
                    Err(rc) => log!("Conversion failed: {0} / {0:?}\n", rc)
                };
            }
        }
        else {
            break;
        }
    }

    Ok(())
}

pub fn crypto_test() -> Result<()> {
    log!("Testing crypto stuff...\n");

    let k_ret_bin = String::from("sdmc:/switch/key_retail.bin");
    let mut k_ret_bin_f = fs::open_file(k_ret_bin, fs::FileOpenOption::Read())?;
    let rks: bin::RetailKeySet = k_ret_bin_f.read_val()?;

    let test_bin = String::from("sdmc:/test_amiibo_rust.bin");
    let mut test_bin_f = fs::open_file(test_bin, fs::FileOpenOption::Read())?;
    let raw_b: bin::RawFormat = test_bin_f.read_val()?;

    let conv_b = bin::ConvertedFormat::from_raw(&raw_b);
    let plain_b = bin::PlainFormat::decrypt_from_converted(&conv_b, &rks)?;

    {
        let dec_bin = String::from("sdmc:/test_amiibo_rust_dec.bin");
        let _ = fs::delete_file(dec_bin.clone());
        let mut dec_bin_f = fs::open_file(dec_bin, fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
        dec_bin_f.write_val(plain_b)?;
    }

    {
        let area_bin = String::from("sdmc:/test_amiibo_rust_dec_area.bin");
        let _ = fs::delete_file(area_bin.clone());
        let mut area_bin_f = fs::open_file(area_bin, fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
        area_bin_f.write_val(plain_b.dec_data.app_area)?;
    }

    log!("Crypto test done!");

    Ok(())
}

pub fn convert_deprecated_virtual_amiibos() {
    log!("Analyzing deprecated dir...\n");
    let _ = convert_deprecated_virtual_amiibos_in_dir(String::from(super::DEPRECATED_VIRTUAL_AMIIBO_DIR));
    log!("Analyzing regular dir...\n");
    let _ = convert_deprecated_virtual_amiibos_in_dir(String::from(super::VIRTUAL_AMIIBO_DIR));

    // crypto_test().unwrap();
}