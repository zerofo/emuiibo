use nx::result::*;
use nx::ipc::sf;
use nx::mii;
use nx::mii::IMiiDatabaseClient;
use alloc::vec;
use alloc::vec::Vec;
use nx::fs;
pub use generic_once_cell::OnceCell;

pub fn initialize() -> Result<()> {
    mii::initialize()
}



const DEFAULT_MII_NAME: &'static str = "emuiibo";

#[inline]
pub fn generate_random_mii() -> Result<mii::CharInfo> {
    let db_guard = mii::get_mii_database();
    let mut char_info = db_guard.as_ref().expect("mii service should be initialized").build_random(sf::EnumAsPrimitiveType::from(mii::Age::All), sf::EnumAsPrimitiveType::from(mii::Gender::All), sf::EnumAsPrimitiveType::from(mii::FaceColor::All))?;
    // Default name is "no name", use our own default instead
    char_info.name.set_str(DEFAULT_MII_NAME);
    Ok(char_info)
}

const MII_SOURCE_FLAG: mii::SourceFlag = mii::SourceFlag::Database();
pub const EXPORTED_MIIS_DIR: &'static str = "sdmc:/emuiibo/miis";

pub fn export_miis() -> Result<()> {
    let db_guard = mii::get_mii_database();
    let db = db_guard.as_ref().expect("mii service should be initialized");
    let mii_count = db.get_count(MII_SOURCE_FLAG)?;
    let mut miis: Vec<mii::CharInfo> = vec![Default::default(); mii_count as usize];

    let mii_total = db.get_one(MII_SOURCE_FLAG, unsafe { sf::OutMapAliasBuffer::from_mut_ptr(miis.as_mut_ptr(), miis.len()) })?;
    for i in 0..mii_total {
        let mii = miis[i as usize];

        let mii_dir_path = format!("{}/{}", EXPORTED_MIIS_DIR, i);
        fs::create_directory(mii_dir_path.as_str())?;

        let mii_path = format!("{}/mii-charinfo.bin", mii_dir_path);
        let mut mii_file = fs::open_file(mii_path.as_str(), fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
        mii_file.write_val::<_, false>(&mii)?;

        let mii_name = format!("{}/name.txt", mii_dir_path);
        let mut mii_name_file = fs::open_file(mii_name.as_str(), fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?;
        let actual_name = mii.name.get_string()?;
        mii_name_file.write_array::<u8, false>(actual_name.as_bytes())?;
    }
    
    Ok(())
}