use nx::result::*;
use nx::ipc::sf;
use nx::service;
use nx::service::mii;
use nx::service::mii::{DatabaseService, IDatabaseService};
use nx::service::mii::IStaticService;
use nx::sync::Mutex;
use nx::mem;
use nx::fs;
use alloc::vec::Vec;

static mut G_STATIC_SRV: Option<Mutex<mii::StaticService>> = None;
static mut G_DB_SRV: Option<Mutex<DatabaseService>> = None;

#[inline]
fn get_static_service() -> Result<&'static Mutex<mii::StaticService>> {
    unsafe {
        G_STATIC_SRV.as_ref().ok_or(nx::rc::ResultNotInitialized::make())
    }
}

#[inline]
fn get_database_service() -> Result<&'static Mutex<DatabaseService>> {
    unsafe {
        G_DB_SRV.as_ref().ok_or(nx::rc::ResultNotInitialized::make())
    }
}

pub fn initialize() -> Result<()> {
    unsafe {
        G_STATIC_SRV = Some(Mutex::new(service::new_service_object()?));
        G_DB_SRV = Some(Mutex::new(get_static_service()?.lock().get_database_service(mii::SpecialKeyCode::Normal)?));
        Ok(())
    }
    
}

pub fn finalize() {
    unsafe {
        G_DB_SRV = None;
        G_STATIC_SRV = None;
    }
}

const DEFAULT_MII_NAME: &'static str = "emuiibo";

#[inline]
pub fn generate_random_mii() -> Result<mii::CharInfo> {
    let mut char_info = get_database_service()?.lock().build_random(sf::EnumAsPrimitiveType::from(mii::Age::All), sf::EnumAsPrimitiveType::from(mii::Gender::All), sf::EnumAsPrimitiveType::from(mii::FaceColor::All))?;
    // Default name is "no name", use our own default instead
    char_info.name.set_str(DEFAULT_MII_NAME);
    Ok(char_info)
}

const MII_SOURCE_FLAG: mii::SourceFlag = mii::SourceFlag::Database();
pub const EXPORTED_MIIS_DIR: &'static str = "sdmc:/emuiibo/miis";

pub fn export_miis() -> Result<()> {
    let mii_count = get_database_service()?.lock().get_count(MII_SOURCE_FLAG)?;
    let miis: Vec<mii::CharInfo> = vec![Default::default(); mii_count as usize];

    let mii_total = get_database_service()?.lock().get_1(MII_SOURCE_FLAG, sf::Buffer::from_array(&miis))?;
    for i in 0..mii_total {
        let mii = miis[i as usize];

        let mii_dir_path = format!("{}/{}", EXPORTED_MIIS_DIR, i);
        fs::create_directory(mii_dir_path.as_str())?;

        let mii_path = format!("{}/mii-charinfo.bin", mii_dir_path);
        let mut mii_file = fs::open_file(mii_path.as_str(), fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
        mii_file.write_val(&mii)?;

        let mii_name = format!("{}/name.txt", mii_dir_path);
        let mut mii_name_file = fs::open_file(mii_name.as_str(), fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
        let actual_name = mii.name.get_string()?;
        mii_name_file.write_array(actual_name.as_bytes())?;
    }
    
    Ok(())
}