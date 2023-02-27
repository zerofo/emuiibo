use alloc::vec::Vec;
use nx::result::*;
use nx::fs;
use alloc::string::String;
use crate::{amiibo, miiext};

#[inline]
pub fn exists_file(path: String) -> bool {
    match fs::get_entry_type(path) {
        Ok(ent_type) => ent_type == fs::DirectoryEntryType::File,
        Err(_) => false
    }
}

pub const BASE_DIR: &'static str = "sdmc:/emuiibo";

pub fn get_path_without_extension(path: String) -> String {
    let mut path_items: Vec<&str> = path.split_terminator('.').collect();
    path_items.pop();
    path_items.join(".")
}

pub fn get_path_file_name(path: String) -> String {
    let path_items: Vec<&str> = path.split_terminator('/').collect();
    String::from(*path_items.last().unwrap_or(&""))
}

#[inline]
pub fn get_path_file_name_without_extension(path: String) -> String {
    get_path_file_name(get_path_without_extension(path))
}

// Note: using macros instead of fns to avoid having to deal with serde_json's lifetime stuff

macro_rules! read_deserialize_json {
    ($path:expr => $t:ty) => {{
        let json_data = {
            let mut file = nx::fs::open_file($path, nx::fs::FileOpenOption::Read())?;
            let mut data: alloc::vec::Vec<u8> = vec![0; file.get_size()?];
            file.read_array(&mut data)?;
            data
        };
    
        let t = match serde_json::from_slice::<$t>(&json_data) {
            Ok(t) => t,
            Err(_) => return Err($crate::rc::ResultInvalidJsonDeserialization::make())
        };
    
        Ok(t.clone())
    }};
}

macro_rules! write_serialize_json {
    ($path:expr, $t:expr) => {
        if let Ok(json_data) = serde_json::to_vec_pretty($t) {
            let _ = nx::fs::delete_file($path.clone());
            let mut file = nx::fs::open_file($path.clone(), nx::fs::FileOpenOption::Create() | nx::fs::FileOpenOption::Write() | nx::fs::FileOpenOption::Append())?;
            file.write_array(&json_data)?;
            Ok(())
        }
        else {
            Err($crate::rc::ResultInvalidJsonSerialization::make())
        }
    };
}

pub fn ensure_directories() -> Result<()> {
    let _ = fs::create_directory(String::from(BASE_DIR));
    let _ = fs::create_directory(String::from(amiibo::VIRTUAL_AMIIBO_DIR));
    fs::clean_directory_recursively(String::from(miiext::EXPORTED_MIIS_DIR))?;

    Ok(())
}