/*

- [v1] 0.1 -> plain raw bins

- [v2] 0.2, 0.2.1 --> {
    amiibo.json: {
        string name
        Date firstWriteDate [u16, u8, u8]
        Date lastWriteDate [...]
        u32 applicationAreaSize
        bool randomizeUuid
    }
    mii.dat (charinfo)
    amiibo.bin (raw dump)
}

- [v3] 0.3, 0.3.1, 0.4 --> {
    common.json: {
        u16 writeCounter
        Date lastWriteDate ("2000-10-10"-like string)
        u16 version
    }
    tag.json: {
        u8[10] uuid
        bool randomUuid
    }
    model.json: {
        u8[8] amiiboId
    }
    register.json: {
        string name
        string miiCharInfo (rel path name)
        Date firstWriteDate
    }
}

// [fmt] 0.5, 0.5.1, 0.6, 0.6.1, 0.6.2, 0.6.3, 0.6.4, 1.0 [+ areas change] --> {
    amiibo.flag
    amiibo.json: {
        string name
        u16 write_counter
        u16 version
        string mii_charinfo_file
        Date first_write_date { u16 y, u8 m, u8 d }
        Date last_write_date {}
        id: {
            u16 game_character_id
            u8 character_variant
            u8 figure_type
            u8 series
            u16 model_number
        }
        u8[10] uuid
    }
}

*/

use nx::{result::*, fs};
use super::{v1, v2, v3, fmt, VirtualAmiiboFormat};
use alloc::string::String;

pub trait DeprecatedVirtualAmiiboFormat: super::VirtualAmiiboFormat {
    fn convert(&self) -> Result<fmt::VirtualAmiibo>;

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
    let mut dir = fs::open_directory(path.clone(), fs::DirectoryOpenMode::ReadDirectories() | fs::DirectoryOpenMode::ReadFiles())?;

    loop {
        if let Some(entry) = dir.read_next()? {
            let entry_path = format!("{}/{:?}", path, entry.name);
            log!("Analyzing entry '{}'...\n", entry_path);

            let maybe_new_amiibo = {
                if let Ok(v1_amiibo) = v1::VirtualAmiibo::try_load(entry_path.clone()) {
                    log!("Loaded v1 amiibo {:?} - converting it...\n", v1_amiibo);
                    Some(v1_amiibo.convert())
                }
                else if let Ok(v2_amiibo) = v2::VirtualAmiibo::try_load(entry_path.clone()) {
                    log!("Loaded v2 amiibo {:?} - converting it...\n", v2_amiibo);
                    Some(v2_amiibo.convert())
                }
                else if let Ok(v3_amiibo) = v3::VirtualAmiibo::try_load(entry_path.clone()) {
                    log!("Loaded v3 amiibo {:?} - converting it...\n", v3_amiibo);
                    Some(v3_amiibo.convert())
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

pub fn convert_deprecated_virtual_amiibos() {
    log!("Analyzing deprecated dir...\n");
    let _ = convert_deprecated_virtual_amiibos_in_dir(String::from(super::DEPRECATED_VIRTUAL_AMIIBO_DIR));
    log!("Analyzing regular dir...\n");
    let _ = convert_deprecated_virtual_amiibos_in_dir(String::from(super::VIRTUAL_AMIIBO_DIR));
}