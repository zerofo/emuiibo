use nx::fs;
use alloc::string::String;
use crate::emu;

pub const LOG_FILE: &'static str = "sdmc:/emuiibo/emuiibo.log";

pub const fn is_logging_enabled() -> bool {
    // Only enable logging on dev mode
    emu::CURRENT_VERSION.is_dev_build
}

pub fn initialize() {
    let _ = fs::delete_file(String::from(LOG_FILE));
}

fn log_impl(line: String) {
    if is_logging_enabled() {
        match fs::open_file(String::from(LOG_FILE), fs::FileOpenOption::Create() | fs::FileOpenOption::Write() | fs::FileOpenOption::Append()) {
            Ok(mut log_file) => {
                let _ = log_file.write(line.as_ptr(), line.len());
            }
            _ => {}
        };
    }
}

pub fn log_line(text: String) {
    log_impl(format!("{}\n", text));
}

pub fn log_line_str(text: &str) {
    log_impl(format!("{}\n", text));
}