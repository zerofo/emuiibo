use crate::fsext;
use alloc::string::String;
use core::sync::atomic::AtomicBool;
use nx::fs;
use nx::fs::FileAccessor;
use nx::result::*;
use nx::sync;

static G_CAN_LOG: AtomicBool = AtomicBool::new(false);
static G_LOG_FILE: sync::Mutex<Option<FileAccessor>> = sync::Mutex::new(None);

const LOG_FLAG: &str = "log";
const LOG_FILE: &str = "sdmc:/emuiibo/emuiibo.log";

pub fn initialize() -> Result<()> {
    let can_log = fsext::has_flag(LOG_FLAG);
    if can_log {
        let _ = fs::remove_file(LOG_FILE);
        *G_LOG_FILE.lock() = Some(fs::open_file(
            LOG_FILE,
            fs::FileOpenOption::Create()
                | fs::FileOpenOption::Write()
                | fs::FileOpenOption::Append(),
        )?);
        G_CAN_LOG.store(true, core::sync::atomic::Ordering::Release);
    }

    Ok(())
}

pub fn log_string(s: String) {
    log_str(s.as_str())
}

#[inline(always)]
pub fn log_str(log_str: &str) {
    if G_CAN_LOG.load(core::sync::atomic::Ordering::Acquire) {
        let _ = G_LOG_FILE
            .lock()
            .as_mut()
            .expect("We only allow logging after the log file has been opened")
            .write_array(log_str.as_bytes());
    }
}

#[macro_export]
macro_rules! log {
    ($msg:literal) => {
        {
            crate::logger::log_str($msg);
        }
    };
    ($fmt:literal, $( $params:expr ),*) => {
        {
            let msg = alloc::format!($fmt, $( $params ),*);
            crate::logger::log_string(msg);
        }
    };
}
