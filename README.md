# emuiibo

> Custom process MitM'ing `nfp:user` services for amiibo emulation (my custom but really extended fork of `nfp-mitm`)

## Usage

Download the latest release and place it on your CFW's `titles` folder (so it would be like `<cfw>/titles/0100000000000352`).

### Amiibo emulation

With emuiibo running, perform the combo `L + R + X` when the title is looking for amiibos, and it should insta-load the first amiibo in the list.

Default amiibo directory is `sd:/emuiibo`. Place your amiibo dumps (must be `*.bin` files) there.

To move to the next amiibo, perform the combo `L + R + Y`.

Emuiibo gets amiibo's data, but the register info (amiibo name, write dates, mii) is auto-generated:

- Name is hardcoded to `emuiibo`.

- Write date is hardcoded to 15th June 2019.

- The mii is accessed from the console's mii database, so the first mii found in the console is used.

### Amiibo dumps

Dumps consist on `*.bin` files, with sizes usually between 540 and 640 bytes. They can be dumped with several tools.

### For developers

This MitM process also hosts a custom service, `nfp:emu`, which can be used to control amiibo swapping and emulation by IPC.

Trying to register the service again (with `smRegisterService`) and failing would mean that the service is present, hence emuiibo is running. Mentioning this as a way to detect whether emuiibo is present:

```cpp
bool IsEmuiiboPresent()
{
    Handle tmph = 0;
    Result rc = smRegisterService(&tmph, "nfp:emu", false, 1);
    if(R_FAILED(rc)) return true;
    smUnregisterService("nfp:emu");
    return false;
}
```

## Credits

- All the persons who contributed to the `nfp-mitm` project before me: *Subv, ogniK, averne, spx01, SciresM*

- libstratosphere libraries (SciresM again)
