use nx::result::*;
use nx::fs;
use alloc::string::String;

#[inline]
pub fn exists_file(path: String) -> bool {
    match fs::get_entry_type(path) {
        Ok(ent_type) => ent_type == fs::DirectoryEntryType::File,
        Err(_) => false
    }
}

pub const BASE_DIR: &'static str = "sdmc:/emuiibo";
pub const VIRTUAL_AMIIBO_DIR: &'static str = "sdmc:/emuiibo/amiibo";
pub const EXPORTED_MIIS_DIR: &'static str = "sdmc:/emuiibo/miis";

pub fn ensure_directories() -> Result<()> {
    let _ = fs::create_directory(String::from(BASE_DIR));
    let _ = fs::create_directory(String::from(VIRTUAL_AMIIBO_DIR));
    fs::clean_directory_recursively(String::from(EXPORTED_MIIS_DIR))?;

    Ok(())
}