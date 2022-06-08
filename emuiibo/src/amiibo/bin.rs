use alloc::string::String;
use alloc::vec::Vec;
use nx::crypto::hmac;
use nx::crypto::sha256;
use nx::crypto::aes;
use nx::crypto::rc;
use nx::util;
use nx::result::*;
use super::ntag::Manufacturer1;
use super::ntag::Manufacturer2;
use super::{ntag, fmt};

pub trait Buffer: Sized {
    fn get_buf(&self) -> &[u8] {
        unsafe {
            core::slice::from_raw_parts(self as *const _ as *const u8, core::mem::size_of::<Self>())
        }
    }

    fn get_buf_mut(&mut self) -> &mut [u8] {
        unsafe {
            core::slice::from_raw_parts_mut(self as *mut _ as *mut u8, core::mem::size_of::<Self>())
        }
    }
}

pub struct DrbgContext {
    pub hmac_key: Vec<u8>,
    pub iteration: u16,
    pub buf: [u8; core::mem::size_of::<u16>() + Self::MAX_SEED_SIZE],
    pub buf_size: usize
}

impl DrbgContext {
    pub const MAX_SEED_SIZE: usize = 480;

    pub fn new(hmac_key: &[u8], seed: &[u8]) -> Result<Self> {
        result_return_unless!(seed.len() <= Self::MAX_SEED_SIZE, rc::ResultInvalidSize);

        let mut ctx = Self {
            hmac_key: hmac_key.to_vec(),
            iteration: 0,
            buf: [0; core::mem::size_of::<u16>() + Self::MAX_SEED_SIZE],
            buf_size: core::mem::size_of::<u16>() + seed.len()
        };

        unsafe {
            core::ptr::copy(seed.as_ptr(), ctx.buf.as_mut_ptr().add(core::mem::size_of::<u16>()), seed.len());
        }

        Ok(ctx)
    }

    pub fn step(&mut self, out_mac: &mut [u8]) -> Result<()> {
        unsafe {
            *(self.buf.as_mut_ptr() as *mut u16) = self.iteration.swap_bytes();
        }
        self.iteration += 1;

        hmac::sha256::calculate_mac(&self.hmac_key, &self.buf[0..self.buf_size], out_mac)?;
        Ok(())
    }

    pub fn gen_bytes(hmac_key: &[u8], seed: &[u8], out_data: &mut [u8]) -> Result<()> {
        let mut ctx = Self::new(hmac_key, seed)?;

        let mut remaining_size = out_data.len();
        let mut cur_buf = out_data.as_mut_ptr();

        let mut tmp_buf = [0u8; sha256::HASH_SIZE];
        while remaining_size > 0 {
            ctx.step(&mut tmp_buf)?;

            let step_size = remaining_size.min(sha256::HASH_SIZE);
            unsafe {
                core::ptr::copy(tmp_buf.as_ptr(), cur_buf, step_size);
                cur_buf = cur_buf.add(step_size);
            }
            remaining_size -= step_size;
        }

        Ok(())
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C)]
pub struct RetailKey {
    pub hmac_key: [u8; 0x10],
    pub phrase: util::CString<0xE>,
    pub reserved: u8,
    pub seed_size: u8,
    pub seed: [u8; Self::MAX_SEED_SIZE],
    pub xor_pad: [u8; 0x20]
}
const_assert!(core::mem::size_of::<RetailKey>() == 80);

impl RetailKey {
    pub const MAX_SEED_SIZE: usize = 0x10;
}

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(C,)]
pub struct DerivedKey {
    aes_key: [u8; 0x10],
    aes_iv: [u8; 0x10],
    hmac_key: [u8; 0x10]
}
const_assert!(core::mem::size_of::<DerivedKey>() == 0x30);

impl Buffer for DerivedKey {}

impl DerivedKey {
    pub fn derive_from(key: &RetailKey, base_seed: &[u8]) -> Result<Self> {
        let mut prepared_seed = [0u8; DrbgContext::MAX_SEED_SIZE];

        let prepared_seed_buf_start = prepared_seed.as_mut_ptr();
        let mut prepared_seed_buf_cur = prepared_seed_buf_start;

        unsafe {
            let phrase_len = key.phrase.c_str.len();
            core::ptr::copy(key.phrase.c_str.as_ptr(), prepared_seed_buf_cur, phrase_len);
            prepared_seed_buf_cur = prepared_seed_buf_cur.add(phrase_len);

            let seed_size = key.seed_size as usize;
            let leading_seed_bytes = RetailKey::MAX_SEED_SIZE - seed_size;
            core::ptr::copy(base_seed.as_ptr(), prepared_seed_buf_cur, leading_seed_bytes);
            prepared_seed_buf_cur = prepared_seed_buf_cur.add(leading_seed_bytes);

            core::ptr::copy(key.seed.as_ptr(), prepared_seed_buf_cur, seed_size);
            prepared_seed_buf_cur = prepared_seed_buf_cur.add(seed_size);

            let seed_step_size = 0x10usize;
            core::ptr::copy(base_seed.as_ptr().add(0x10), prepared_seed_buf_cur, seed_step_size);
            prepared_seed_buf_cur = prepared_seed_buf_cur.add(seed_step_size);

            for i in 0..key.xor_pad.len() {
                *prepared_seed_buf_cur.add(i) = base_seed[0x20 + i] ^ key.xor_pad[i];
            }
            prepared_seed_buf_cur = prepared_seed_buf_cur.add(key.xor_pad.len());

            let prepared_seed_size = prepared_seed_buf_cur.offset_from(prepared_seed_buf_start) as usize;
            let mut derived_key: Self = Default::default();
            DrbgContext::gen_bytes(&key.hmac_key, &prepared_seed[0..prepared_seed_size], derived_key.get_buf_mut())?;

            Ok(derived_key)
        }
    }
}

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
    #[inline]
    pub fn derive_from(key_set: &RetailKeySet, base_seed: &[u8]) -> Result<Self> {
        Ok(Self {
            data_key: DerivedKey::derive_from(&key_set.data_key, base_seed)?,
            tag_key: DerivedKey::derive_from(&key_set.tag_key, base_seed)?
        })
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

impl Buffer for KeyDerivationSeed {}

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

define_bit_enum! {
    Flags (u8) {
        None = 0,
        // TODO: more flags here? bits 0-3 are probably for internal use as they get masked out in both 3DS and here...
        Initialized = bit!(4),
        ApplicationAreaUsed = bit!(5)
    }
}

#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
#[repr(u8)]
#[allow(dead_code)]
pub enum CountryCode {
    #[default]
    Invalid = 0,

    // Note: https://wiibrew.org/wiki/Country_Codes

    // Japan
    Japan = 1,

    // Americas
    Anguilla = 8,
    AntiguaAndBarbuda = 9,
    Argentina = 10,
    Aruba = 11,
    Bahamas = 12,
    Barbados = 13,
    Belize = 14,
    Bolivia = 15,
    Brazil = 16,
    BritishVirginIslands = 17,
    Canada = 18,
    CaymanIslands = 19,
    Chile = 20,
    Colombia = 21,
    CostaRica = 22,
    Dominica = 23,
    DominicanRepublic = 24,
    Ecuador = 25,
    ElSalvador = 26,
    FrenchGuiana = 27,
    Grenada = 28,
    Guadeloupe = 29,
    Guatemala = 30,
    Guyana = 31,
    Haiti = 32,
    Honduras = 33,
    Jamaica = 34,
    Martinique = 35,
    Mexico = 36,
    Monsterrat = 37,
    NetherlandsAntilles = 38,
    Nicaragua = 39,
    Panama = 40,
    Paraguay = 41,
    Peru = 42,
    StKittsAndNevis = 43,
    StLucia = 44,
    StVincentAndTheGrenadines = 45,
    Suriname = 46,
    TrinidadAndTobago = 47,
    TurksAndCaicosIslands = 48,
    UnitedStates = 49,
    Uruguay = 50,
    USVirginIslands = 51,
    Venezuela = 52,

    // Europe & Africa
    Albania = 64,
    Australia = 65,
    Austria = 66,
    Belgium = 67,
    BosniaAndHerzegovina = 68,
    Botswana = 69,
    Bulgaria = 70,
    Croatia = 71,
    Cyprus = 72,
    CzechRepublic = 73,
    Denmark = 74,
    Estonia = 75,
    Finland = 76,
    France = 77,
    Germany = 78,
    Greece = 79,
    Hungary = 80,
    Iceland = 81,
    Ireland = 82,
    Italy = 83,
    Latvia = 84,
    Lesotho = 85,
    Lichtenstein = 86,
    Lithuania = 87,
    Luxembourg = 88,
    NorthMacedonia = 89,
    Malta = 90,
    Montenegro = 91,
    Mozambique = 92,
    Namibia = 93,
    Netherlands = 94,
    NewZealand = 95,
    Norway = 96,
    Poland = 97,
    Portugal = 98,
    Romania = 99,
    Russia = 100,
    Serbia = 101,
    Slovakia = 102,
    Slovenia = 103,
    SouthAfrica = 104,
    Spain = 105,
    Swaziland = 106,
    Sweden = 107,
    Switzerland = 108,
    Turkey = 109,
    UnitedKingdom = 110,
    Zambia = 111,
    Zimbabwe = 112,
    Azerbaijan = 113,
    Mauritania = 114,
    Mali = 115,
    Niger = 116,
    Chad = 117,
    Sudan = 118,
    Eritrea = 119,
    Djibouti = 120,
    Somalia = 121,

    // Southeast Asia
    Taiwan = 128,
    SouthKorea = 136,
    HongKong = 144,
    Macao = 145,
    Indonesia = 152,
    Singapore = 153,
    Thailand = 154,
    Philippines = 155,
    Malaysia = 156,
    China = 160,

    // Middle East
    UnitedArabEmirates = 168,
    India = 169,
    Egypt = 170,
    Oman = 171,
    Qatar = 172,
    Kuwait = 173,
    SaudiArabia = 174,
    Syria = 175,
    Bahrain = 176,
    Jordan = 177
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
            country_code: CountryCode::Spain,
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

impl Buffer for EncryptedData {}

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

impl Buffer for DecryptedData {}

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
        // Sets everything except for encrypted sections
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

    pub fn decrypt_from_converted(conv: &ConvertedFormat, key_set: &RetailKeySet) -> Result<Self> {
        // Convert non-encrypted parts
        let mut plain_bin = Self::from_converted(conv);

        // Decrypt the rest
        let base_seed = KeyDerivationSeed::from_converted(conv);

        let derived_key_set = DerivedKeySet::derive_from(key_set, base_seed.get_buf())?;

        let mut aes_ctr_ctx = aes::ctr::a128::Context::new(&derived_key_set.data_key.aes_key, &derived_key_set.data_key.aes_iv)?;
        aes_ctr_ctx.crypt(conv.enc_data.get_buf(), plain_bin.dec_data.get_buf_mut())?;

        let tag_data_start = &plain_bin.man_info_1 as *const _ as *const u8;
        let tag_data_size = core::mem::size_of::<Manufacturer1>() + core::mem::size_of::<Struct1>();
        let tag_data = unsafe {
            core::slice::from_raw_parts(tag_data_start, tag_data_size)
        };
        hmac::sha256::calculate_mac(&key_set.tag_key.hmac_key, tag_data, &mut plain_bin.tag_sha256_hmac_hash)?;

        let data_data_start = &plain_bin.st_2.unk_1 as *const _ as *const u8;
        let data_data_size = core::mem::size_of::<PlainFormat>() - core::mem::size_of::<Manufacturer2>() - sha256::HASH_SIZE - core::mem::size_of::<u8>();
        let data_data = unsafe {
            core::slice::from_raw_parts(data_data_start, data_data_size)
        };
        hmac::sha256::calculate_mac(&key_set.data_key.hmac_key, data_data, &mut plain_bin.data_sha256_hmac_hash)?;

        // assert_eq!(plain_bin.tag_sha256_hmac_hash, conv.tag_sha256_hmac_hash);
        // assert_eq!(plain_bin.data_sha256_hmac_hash, conv.data_sha256_hmac_hash);

        Ok(plain_bin)
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
                    let name_str = name_be.get_string()?;
                    if name_str.is_empty() {
                        String::from("emuiibo")
                    }
                    else {
                        name_str
                    }
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