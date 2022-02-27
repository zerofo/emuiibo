pub const RESULT_MODULE: u32 = 352;

result_define_group!(RESULT_MODULE => {
    VirtualAmiiboFlagNotFound: 1,
    VirtualAmiiboJsonNotFound: 2,
    InvalidVirtualAmiiboJsonSerialization: 3,
    InvalidVirtualAmiiboJsonDeserialization: 4,
    InvalidLoadedVirtualAmiibo: 5,
    VirtualAmiiboAreasJsonNotFound: 6,
    InvalidActiveVirtualAmiibo: 7,
    InvalidVirtualAmiiboAccessId: 8
});