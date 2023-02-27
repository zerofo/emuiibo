use nx::result::*;
use nx::fs;
use nx::ipc::sf::nfp;
use alloc::string::String;
use crate::amiibo;
use crate::fsext;

pub struct ApplicationArea {
    area_file: String
}

impl ApplicationArea {
    pub fn new() -> Self {
        Self { area_file: String::new() }
    }

    pub fn from(virtual_amiibo: &amiibo::fmt::VirtualAmiibo, access_id: nfp::AccessId) -> Self {
        let areas_dir = format!("{}/areas", virtual_amiibo.path);
        let _ = fs::create_directory(areas_dir.clone());
        Self { area_file: format!("{}/0x{:08X}.bin", areas_dir, access_id) }
    }

    pub fn delete(&mut self) -> Result<()> {
        if self.is_valid() {
            fs::delete_file(self.area_file.clone())?;
            self.area_file.clear();
        }

        Ok(())
    }

    pub fn is_valid(&self) -> bool {
        !self.area_file.is_empty()
    }

    pub fn exists(&self) -> bool {
        if self.is_valid() {
            fsext::exists_file(self.area_file.clone())
        }
        else {
            false
        }
    }

    pub fn create(&self, data: *const u8, data_size: usize, _recreate: bool) -> Result<()> {
        // TODO: difference between create and recreate commands?
        // write already overwrites the area file
        self.write(data, data_size)
    }

    pub fn write(&self, data: *const u8, data_size: usize) -> Result<()> {
        let _ = fs::delete_file(self.area_file.clone());
        let mut file = fs::open_file(self.area_file.clone(), fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
        file.write(data, data_size)?;
        Ok(())
    }

    pub fn read(&self, data: *mut u8, data_size: usize) -> Result<()> {
        let mut file = fs::open_file(self.area_file.clone(), fs::FileOpenOption::Read())?;
        file.read(data, data_size)?;
        Ok(())
    }

    pub fn get_size(&self) -> Result<usize> {
        let mut file = fs::open_file(self.area_file.clone(), fs::FileOpenOption::Read())?;
        file.get_size()
    }
}