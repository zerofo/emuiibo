use alloc::string::String;
use alloc::string::ToString;
use nx::fs;
use nx::result::*;
use crate::fsext;

static mut G_CAN_LOG: bool = false;
static mut G_LOG_FILE: Option<fs::FileAccessor> = None;

const LOG_FLAG: &str = "log";
const LOG_FILE: &str = "sdmc:/emuiibo/emuiibo.log";

pub fn initialize() -> Result<()> {
    unsafe {
        G_CAN_LOG = fsext::has_flag(LOG_FLAG);

        if G_CAN_LOG {
            let _ = fs::delete_file(LOG_FILE.to_string());
            G_LOG_FILE = Some(fs::open_file(LOG_FILE.to_string(), fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append())?);
        }
    }

    Ok(())
}

pub fn log_string(log_str: String) {
    unsafe {
        if G_CAN_LOG {
            let _ = G_LOG_FILE.as_mut().unwrap().write_array(log_str.as_bytes());
        }
    }
}

#[inline(always)]
pub fn log_str(log_str: &str) {
    log_string(log_str.to_string());
}

macro_rules! log {
    ($msg:literal) => {
        {
            crate::logger::log_str($msg);
        }
    };
    ($fmt:literal, $( $params:expr ),*) => {
        {
            let msg = format!($fmt, $( $params ),*);
            crate::logger::log_string(msg);
        }
    };
}