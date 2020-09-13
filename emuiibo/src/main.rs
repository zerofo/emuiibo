#![no_std]
#![no_main]

#![feature(global_asm)]

#[macro_use]
extern crate nx;

#[macro_use]
extern crate alloc;

extern crate paste;

use nx::result::*;
use nx::results;
use nx::util;
use nx::thread;
use nx::diag::assert;
use nx::ipc::server;
use nx::fs;
use core::panic;

mod resultsext;
mod ipc;
mod emu;
mod fsext;
mod amiibo;
mod area;

static mut STACK_HEAP: [u8; 0x20000] = [0; 0x20000];

#[no_mangle]
pub fn initialize_heap(_hbl_heap: util::PointerAndSize) -> util::PointerAndSize {
    unsafe {
        util::PointerAndSize::new(STACK_HEAP.as_mut_ptr(), STACK_HEAP.len())
    }
}

const POINTER_BUF_SIZE: usize = 0x1000;
type Manager = server::ServerManager<POINTER_BUF_SIZE>;

#[no_mangle]
pub fn main() -> Result<()> {
    thread::get_current_thread().name.set_str("emuiibo.Main")?;
    fs::initialize()?;
    fs::mount_sd_card("sdmc")?;
    fsext::ensure_directories();

    let mut manager = Manager::new();
    manager.register_mitm_service_server::<ipc::nfp::UserManager>()?;
    manager.register_service_server::<ipc::emu::EmulationService>()?;
    manager.loop_process()?;

    fs::finalize();
    Ok(())
}

#[panic_handler]
fn panic_handler(info: &panic::PanicInfo) -> ! {
    util::on_panic_handler::<nx::diag::log::LmLogger>(info, assert::AssertMode::FatalThrow, results::lib::assert::ResultAssertionFailed::make())
}