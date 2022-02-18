macro_rules! log {
    ($msg:literal) => {
        {
            if crate::emu::CURRENT_VERSION.is_dev_build {
                diag_log!(nx::diag::log::LmLogger { nx::diag::log::LogSeverity::Trace, false } => $msg);
            }
        }
    };
    ($fmt:literal, $( $params:expr ),*) => {
        {
            if crate::emu::CURRENT_VERSION.is_dev_build {
                diag_log!(nx::diag::log::LmLogger { nx::diag::log::LogSeverity::Trace, false } => $fmt, $( $params ),*);
            }
        }
    };
}