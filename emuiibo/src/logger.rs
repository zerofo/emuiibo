use alloc::string::String;
use crate::fsext;

static mut G_CAN_LOG: bool = false;

const LOG_FLAG_FILE: &str = "sdmc:/emuiibo/flags/log.flag";

pub fn initialize() {
    unsafe {
        G_CAN_LOG = fsext::exists_file(String::from(LOG_FLAG_FILE));
    }
}

#[inline(always)]
pub fn can_log() -> bool {
    unsafe {
        G_CAN_LOG
    }
}

macro_rules! log {
    ($msg:literal) => {
        {
            if crate::logger::can_log() {
                diag_log!(nx::diag::log::lm::LmLogger { nx::diag::log::LogSeverity::Trace, false } => $msg);
            }
        }
    };
    ($fmt:literal, $( $params:expr ),*) => {
        {
            if crate::logger::can_log() {
                diag_log!(nx::diag::log::lm::LmLogger { nx::diag::log::LogSeverity::Trace, false } => $fmt, $( $params ),*);
            }
        }
    };
}