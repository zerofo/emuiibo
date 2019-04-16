# emuiibo

> Custom process MitM'ing `nfp:user` services for amiibo emulation (my custom but really extended fork of `nfp-mitm`)

## Usage

Download the latest release and place it on your CFW's `titles` folder (so it would be like `<cfw>/titles/0100000000000352`).

You also have to set the boot2 flag:´

- Atmosphere: create a file named `boot2.flag` inside `titles/0100000000000352/flags` directory.

- ReiNX: same as above, but in the root title dir (`titles/0100000000000352`).

### Amiibo emulation

With emuiibo running, perform the combo `L + R + X` when the title is looking for amiibos, and it should insta-load the first amiibo in the list.

Default amiibo directory is `sd:/emuiibo`. Place your amiibo dumps (must be `*.bin` files) there.

To move to the next amiibo, perform the combo `L + R + Y`.

Emuiibo gets amiibo's data, but the register info (amiibo name, write dates, mii) is auto-generated, as it isn't present on amiibo dumps:

- Name will be the file's name (`Amiibo.bin` -> `Amiibo`), but if the name is bigger than 10 chars it will be hardcoded to `Emuiibo`.

- Write date is hardcoded to 15th June 2019.

- The mii is accessed from the console's mii database, so the first mii found in the console is used.

### Amiibo dumps

Dumps consist on `*.bin` files, which must be 540 bytes (perhaps even more?). They can be dumped with several tools.

### For developers

This MitM process also hosts a custom service, `nfp:emu`, which can be used to control amiibo swapping and emulation by IPC.

You have an implementation for C/C++ and libnx in [here](nfpemu-libnx).

## Credits

- All the persons who contributed to the `nfp-mitm` project before me: *Subv, ogniK, averne, spx01, SciresM*

- libstratosphere libraries (SciresM again)