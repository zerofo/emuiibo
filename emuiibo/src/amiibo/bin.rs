use alloc::string::String;
use nx::crypto::sha256;
use nx::util;
use nx::result::*;
use super::{ntag, fmt};

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct RetailKey {
    pub hmac_key: [u8; 0x10],
    pub phrase: util::CString<0xE>,
    pub reserved: u8,
    pub seed_size: u8,
    pub seed: [u8; 0x10],
    pub xor_pad: [u8; 0x20]
}
const_assert!(core::mem::size_of::<RetailKey>() == 80);

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C,)]
pub struct DerivedKey {
    aes_key: [u8; 0x10],
    aes_iv: [u8; 0x10],
    hmac_key: [u8; 0x10]
}
const_assert!(core::mem::size_of::<DerivedKey>() == 0x30);

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct RetailKeySet {
    pub data_key: RetailKey,
    pub tag_key: RetailKey
}
const_assert!(core::mem::size_of::<RetailKeySet>() == 2 * core::mem::size_of::<RetailKey>());

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct DerivedKeySet {
    pub data_key: DerivedKey,
    pub tag_key: DerivedKey
}
const_assert!(core::mem::size_of::<DerivedKeySet>() == 2 * core::mem::size_of::<DerivedKey>());

impl DerivedKeySet {
    pub fn derive_from(key_set: &RetailKeySet) -> Self {
        todo!("Derive keys from retail");
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct KeyDerivationSeed {
    pub unk_1: u8,
    pub write_counter: u8,
    pub pad_zero: [u8; 0xE],
    pub man_info_1s: [ntag::Manufacturer1; 2],
    pub unk_maybe_hash: [u8; 0x20]
}
const_assert!(core::mem::size_of::<KeyDerivationSeed>() == 0x40);

impl KeyDerivationSeed {
    pub const fn from_converted(conv: &ConvertedFormat) -> Self {
        Self {
            unk_1: conv.st_2.unk_1,
            write_counter: conv.st_2.write_counter,
            pad_zero: [0; 0xE],
            man_info_1s: [conv.man_info_1; 2],
            unk_maybe_hash: conv.st_1.unk_maybe_hash
        }
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct AmiiboId {
    pub game_character_id: u16,
    pub character_variant: u8,
    pub figure_type: u8,
    pub model_number: u16,
    pub series: u8,
    pub unk_0x2: u8
}
const_assert!(core::mem::size_of::<AmiiboId>() == 0x8);

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct Struct1 {
    pub amiibo_id: AmiiboId,
    pub unk: [u8; 0x4],
    pub unk_maybe_hash: [u8; 0x20]
}
const_assert!(core::mem::size_of::<Struct1>() == 0x2C);

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct Struct2 {
    pub unk_0xa5: u8,
    pub unk_1: u8,
    pub write_counter: u8,
    pub unk_2: u8
}
const_assert!(core::mem::size_of::<Struct2>() == 0x4);

bit_enum! {
    Flags (u8) {
        None = 0,
        // TODO: more flags here? bits 0-3 are probably for internal use as they get masked out in both 3DS and here...
        Initialized = bit!(4),
        ApplicationAreaUsed = bit!(5)
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(u8)]
pub enum CountryCode {
    #[default]
    Todo = 0
    // Same format as DSi and Wii, unused in this console but present on amiibos
    // TODO: fill in with values (list: https://wiibrew.org/wiki/Country_Codes)
}

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct Date {
    pub val_be: u16
}

impl Date {
    pub const fn new(year: u16, month: u8, day: u8) -> Self {
        let mut val_be: u16 = 0;
        write_bits!(0, 4, val_be, day as u16);
        write_bits!(5, 8, val_be, month as u16);
        write_bits!(9, 15, val_be, year - 2000);
        Self { val_be }
    }

    pub const fn get_day(&self) -> u8 {
        (read_bits!(0, 4, self.val_be)) as u8
    }

    pub const fn get_month(&self) -> u8 {
        (read_bits!(5, 8, self.val_be)) as u8
    }

    pub const fn get_year(&self) -> u16 {
        read_bits!(9, 15, self.val_be) + 2000
    }

    pub const fn to_virtual_amiibo_date(&self) -> fmt::VirtualAmiiboDate {
        fmt::VirtualAmiiboDate {
            y: self.get_year(),
            m: self.get_month(),
            d: self.get_day()
        }
    }
}
const_assert!(core::mem::size_of::<Date>() == 0x2);

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
#[repr(C)]
pub struct MiiFormat {
    // Note: 3DS mii format
    // TODO: fill in (info: https://www.3dbrew.org/wiki/Mii_Maker#Mii_QR_Code_format)
    pub data: [u8; 0x60]
}

impl Default for MiiFormat {
    fn default() -> Self {
        Self {
            data: [0; 0x60]
        }
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
#[repr(C, packed)]
pub struct Settings {
    pub flags: Flags,
    pub country_code: CountryCode,
    pub crc32_write_counter_be: u16,
    pub first_write_date_be: Date,
    pub last_write_date_be: Date,
    pub crc32_be: u32,
    pub name_be: util::CString16<10>,
    pub mii: MiiFormat,
    pub program_id_be: u64,
    pub write_counter_be: u16,
    pub access_id_be: u32,
    pub unk: [u8; 0x2],
    pub unk_maybe_hash: [u8; 0x20]
}
const_assert!(core::mem::size_of::<Settings>() == 0xB0);

impl Default for Settings {
    fn default() -> Self {
        Self {
            flags: Flags::None(),
            country_code: CountryCode::Todo,
            crc32_write_counter_be: 0,
            first_write_date_be: Date::new(2001, 9, 11),
            last_write_date_be: Date::new(2001, 9, 11),
            crc32_be: 0,
            name_be: util::CString16::from_str("emuiibo").unwrap().swap_chars(),
            mii: Default::default(),
            program_id_be: 0,
            write_counter_be: 0,
            access_id_be: 0,
            unk: [0; 0x2],
            unk_maybe_hash: [0; 0x20]
        }
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
#[repr(C)]
pub struct EncryptedData {
    pub section_1: [u8; 0x20],
    pub section_2: [u8; 0x114],
    pub section_3: [u8; 0x54]
}
const_assert!(core::mem::size_of::<EncryptedData>() == 0x188);

impl Default for EncryptedData {
    fn default() -> Self {
        Self {
            section_1: [0; 0x20],
            section_2: [0; 0x114],
            section_3: [0; 0x54]
        }
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
#[repr(C)]
pub struct DecryptedData {
    pub settings: Settings,
    pub app_area: [u8; super::APP_AREA_SIZE]
}
const_assert!(core::mem::size_of::<EncryptedData>() == core::mem::size_of::<DecryptedData>());

impl Default for DecryptedData {
    fn default() -> Self {
        Self {
            settings: Default::default(),
            app_area: [0; super::APP_AREA_SIZE]
        }
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
#[repr(C)]
pub struct RawFormat {
    pub man_info_1: ntag::Manufacturer1,
    pub man_info_2: ntag::Manufacturer2,
    pub st_2: Struct2,
    pub enc_section_1: [u8; 0x20],
    pub tag_sha256_hmac_hash: [u8; sha256::HASH_SIZE],
    pub st_1: Struct1,
    pub data_sha256_hmac_hash: [u8; sha256::HASH_SIZE],
    pub enc_section_2: [u8; 0x114],
    pub enc_section_3: [u8; 0x54],
    pub dyn_lock: ntag::DynamicLock,
    pub cfg: ntag::Config
}
const_assert!(core::mem::size_of::<RawFormat>() == 532);

impl Default for RawFormat {
    fn default() -> Self {
        Self {
            man_info_1: Default::default(),
            man_info_2: Default::default(),
            st_2: Default::default(),
            enc_section_1: Default::default(),
            tag_sha256_hmac_hash: Default::default(),
            st_1: Default::default(),
            data_sha256_hmac_hash: Default::default(),
            enc_section_2: [0; 0x114],
            enc_section_3: [0; 0x54],
            dyn_lock: Default::default(),
            cfg: Default::default()
        }
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
#[repr(C)]
pub struct ConvertedFormat {
    pub man_info_2: ntag::Manufacturer2,
    pub data_sha256_hmac_hash: [u8; sha256::HASH_SIZE],
    pub st_2: Struct2,
    pub enc_data: EncryptedData,
    pub tag_sha256_hmac_hash: [u8; sha256::HASH_SIZE],
    pub man_info_1: ntag::Manufacturer1,
    pub st_1: Struct1
}
const_assert!(core::mem::size_of::<ConvertedFormat>() == 520);

impl ConvertedFormat {
    pub const fn from_raw(raw: &RawFormat) -> Self {
        Self {
            man_info_2: raw.man_info_2,
            data_sha256_hmac_hash: raw.data_sha256_hmac_hash,
            st_2: raw.st_2,
            enc_data: EncryptedData {
                section_1: raw.enc_section_1,
                section_2: raw.enc_section_2,
                section_3: raw.enc_section_3
            },
            tag_sha256_hmac_hash: raw.tag_sha256_hmac_hash,
            man_info_1: raw.man_info_1,
            st_1: raw.st_1
        }
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug)]
#[repr(C)]
pub struct PlainFormat {
    pub man_info_2: ntag::Manufacturer2,
    pub data_sha256_hmac_hash: [u8; sha256::HASH_SIZE],
    pub st_2: Struct2,
    pub dec_data: DecryptedData,
    pub tag_sha256_hmac_hash: [u8; sha256::HASH_SIZE],
    pub man_info_1: ntag::Manufacturer1,
    pub st_1: Struct1
}
const_assert!(core::mem::size_of::<ConvertedFormat>() == core::mem::size_of::<PlainFormat>());

impl PlainFormat {
    pub fn from_converted(conv: &ConvertedFormat) -> Self {
        // Sets everything except for decrypted sections
        Self {
            man_info_2: conv.man_info_2,
            data_sha256_hmac_hash: conv.data_sha256_hmac_hash,
            st_2: conv.st_2,
            dec_data: Default::default(),
            tag_sha256_hmac_hash: conv.tag_sha256_hmac_hash,
            man_info_1: conv.man_info_1,
            st_1: conv.st_1
        }
    }

    pub fn decrypt_from_converted(conv: &ConvertedFormat, key_set: &DerivedKeySet) -> Self {
        // Convert non-encrypted parts
        // let plain_bin = Self::from_converted(conv);

        // Decrypt the remaining parts
        todo!("AES-128-CTR, SHA256-HMAC support (to handle encrypted sections)");
    }

    pub fn to_virtual_amiibo(&self, path: String, mii_charinfo_file: String) -> Result<fmt::VirtualAmiibo> {
        // Note: doing all the apparently redundant let-s to silence packed-field-access warnings 

        Ok(fmt::VirtualAmiibo {
            info: fmt::VirtualAmiiboInfo {
                first_write_date: {
                    let first_write_date_be = self.dec_data.settings.first_write_date_be;
                    first_write_date_be.to_virtual_amiibo_date()
                },
                id: fmt::VirtualAmiiboId {
                    game_character_id: self.st_1.amiibo_id.game_character_id,
                    character_variant: self.st_1.amiibo_id.character_variant,
                    figure_type: self.st_1.amiibo_id.figure_type,
                    model_number: self.st_1.amiibo_id.model_number.swap_bytes(),
                    series: self.st_1.amiibo_id.series
                },
                last_write_date: {
                    let last_write_date_be = self.dec_data.settings.last_write_date_be;
                    last_write_date_be.to_virtual_amiibo_date()
                },
                mii_charinfo_file,
                name: {
                    let name_be = self.dec_data.settings.name_be;
                    name_be.swap_chars().get_string()?
                },
                uuid: Some(vec! [
                    self.man_info_1.uid_p1[0],
                    self.man_info_1.uid_p1[1],
                    self.man_info_1.uid_p1[2],
                    self.man_info_1.check_byte_1,
                    self.man_info_1.uid_p2[0],
                    self.man_info_1.uid_p2[1],
                    self.man_info_1.uid_p2[2],
                    self.man_info_1.uid_p2[3],
                    self.man_info_2.check_byte_2,
                    self.man_info_2.internal
                ]),
                version: 0,
                write_counter: {
                    let write_counter_be = self.dec_data.settings.write_counter_be;
                    write_counter_be.swap_bytes()
                }
            },
            mii_charinfo: Default::default(), // TODO: convert from 3DS format!
            areas: {
                let flags = self.dec_data.settings.flags; 
                let access_id_be = self.dec_data.settings.access_id_be;
                let program_id_be = self.dec_data.settings.program_id_be;

                match flags.contains(Flags::ApplicationAreaUsed()) {
                    true => fmt::VirtualAmiiboAreaInfo {
                        areas: vec! [
                            fmt::VirtualAmiiboAreaEntry {
                                program_id: program_id_be.swap_bytes(),
                                access_id: access_id_be.swap_bytes()
                            }
                        ],
                        current_area_access_id: access_id_be.swap_bytes()
                    },
                    false => fmt::VirtualAmiiboAreaInfo::empty()
                }
            },
            path
        })
    }
}