#![no_std]
#![no_main]
#![feature(const_trait_impl)]
#![feature(try_blocks)]

#[macro_use]
extern crate nx;

#[macro_use]
extern crate static_assertions;

#[macro_use]
extern crate alloc;

extern crate paste;

use nx::result::*;
use nx::util;
use nx::thread;
use nx::diag::abort;
use nx::diag::log;
use nx::ipc::server;
use nx::fs;
use nx::service::applet;
use core::panic;
use core::sync::atomic::AtomicBool;

#[macro_use]
mod logger;

mod rc;

#[macro_use]
mod fsext;

mod miiext;

mod ipc;

mod emu;

mod amiibo;

mod area;

mod rand;

rrt0_define_default_module_name!();

const CUSTOM_HEAP_SIZE: usize = 0x8000;
static mut CUSTOM_HEAP: [u8; CUSTOM_HEAP_SIZE] = [0; CUSTOM_HEAP_SIZE];

#[no_mangle]
pub fn initialize_heap(_hbl_heap: util::PointerAndSize) -> util::PointerAndSize {
    unsafe {
        util::PointerAndSize::new(CUSTOM_HEAP.as_mut_ptr(), CUSTOM_HEAP.len())
    }
}

#[no_mangle]
pub fn main() -> Result<()> {
    //applet::initialize();

    fs::initialize_fspsrv_session()?;
    fs::mount_sd_card("sdmc")?;

    fsext::ensure_directories()?;
    if let Err(rc) = logger::initialize() {
        let _a = rc;
    }
    log!("Hello world!\n");

    if let Err(e) = rand::initialize() {
        log!("Error initlializing rand provider: {:?}", e);
        return Ok(());
    }

    if let Err(e) = miiext::initialize()  {
        log!("Error initlializing mii module provider: {:?}", e);
        rand::finalize();
        return Ok(());
    }

    if let Err(e)  = miiext::export_miis() {
        log!("Error exporting mii module provider: {:?}", e);
        rand::finalize();
        miiext::finalize();
        return Ok(());
    }

    amiibo::compat::convert_deprecated_virtual_amiibos();
    emu::load_emulation_status();

    if let Err(e) = ipc::nfp::initialize() {
        log!("Error initializing nfp module provider: {:?}", e);
        rand::finalize();
        miiext::finalize();
        return Ok(());
    }

    const POINTER_BUF_SIZE: usize = 0x1000;
    type Manager = server::ServerManager<POINTER_BUF_SIZE>;

    let mut manager = Manager::new()?;
    let res: nx::result::Result<()> = try {
        manager.register_mitm_service_server::<ipc::nfp::user::UserManager>()?;
        manager.register_mitm_service_server::<ipc::nfp::sys::SystemManager>()?;
        manager.register_service_server::<ipc::emu::EmulationServer>()?;
        manager.loop_process()?;
    };
    
    if let Err(e)= res {
        log!("Error running the server manager: {:?}", e);
    }

    
    miiext::finalize();
    rand::finalize();
    fs::unmount_all();
    Ok(())
}

#[panic_handler]
fn panic_handler(info: &panic::PanicInfo) -> ! {
    util::simple_panic_handler::<log::lm::LmLogger>(info, abort::AbortLevel::SvcBreak())
}
