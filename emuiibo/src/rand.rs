use nx::rand;
use nx::result::*;

static mut G_RNG: Option<rand::SplCsrngGenerator> = None;

pub fn initialize() -> Result<()> {
    unsafe {
        G_RNG = Some(rand::SplCsrngGenerator::new()?);
    }

    Ok(())
}

pub fn finalize() {
    unsafe {
        G_RNG = None;
    }
}

#[inline]
pub fn get_rng() -> Result<&'static mut rand::SplCsrngGenerator> {
    unsafe {
        G_RNG.as_mut().ok_or(nx::rc::ResultNotInitialized::make())
    }
}