use alloc::string::String;
use nx::result::*;
use nx::rand::RandomGenerator;
use crate::rand;

pub mod compat;

pub mod ntag;

pub mod bin;

pub mod v1;

pub mod v2;

pub mod v3;

pub mod fmt;

pub const APP_AREA_SIZE: usize = 0xD8;

pub const VIRTUAL_AMIIBO_DIR: &'static str = "sdmc:/emuiibo/amiibo";
pub const DEPRECATED_VIRTUAL_AMIIBO_DIR: &'static str = "sdmc:/emuiibo";

pub trait VirtualAmiiboFormat {
    fn try_load(path: String) -> Result<Self> where Self: Sized;
}

pub fn generate_random_uuid(uuid: &mut [u8]) -> Result<()> {
    rand::get_rng()?.random_bytes(uuid.as_mut_ptr(), 7)?;
    uuid[7] = 0;
    uuid[8] = 0;
    uuid[9] = 0;
    Ok(())
}