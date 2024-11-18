use nx::rand;
use nx::result::*;
use nx::sync::Mutex;

static G_RNG: Mutex<Option<rand::SplCsrngGenerator>> = Mutex::new(None);

pub fn initialize() -> Result<()> {
        *G_RNG.lock() = Some(rand::SplCsrngGenerator::new()?);

    Ok(())
}

pub fn finalize() {
        *G_RNG.lock() = None;
}

#[inline]
pub fn get_rng() -> Result<rand::SplCsrngGenerator> {
        G_RNG.lock().clone().ok_or(nx::rc::ResultNotInitialized::make())
}