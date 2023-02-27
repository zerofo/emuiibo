use alloc::string::String;
use nx::result::*;

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