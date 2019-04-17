# emuiibo

> Custom process MitM'ing `nfp:user` services for amiibo emulation (my custom but really extended fork of `nfp-mitm`)

## Usage

Download the latest release and place it on your CFW's `titles` folder (so it would be like `<cfw>/titles/0100000000000352`).

According to tests, should work on any CFW which allows NSP sysmodules (Atmosphere, ReiNX).

You also have to set the boot2 flag, whose location depends on the CFW:

- Atmosphere: create a file named `boot2.flag` inside `titles/0100000000000352/flags` directory.

- ReiNX: create a file named `boot2.flag` inside `titles/0100000000000352` directory.

### Combos

All the input combos are performed with R-Stick pressing and pressing the D-pad in an specific direction (at the same time). Combos must (should) be done before or after the game starts looking for amiibos.

- **Toggle amiibo emulation**: Press R-Stick (like it was a button) and also pressing the D-pad up. Toggles/untoggles emulation.

- **Toggle amiibo emulation once**: Same as above, but pressing the D-pad right. Toggles emulation once, after emulating an amiibo then it will untoggle automatically.

- **Untoggle amiibo emulation**: Same as above, but pressing the D-pad down. Untoggles amiibo emulation, and should be used as a way to fully ensure it is untoggled, in case you don't know whether it's toggled or not.

- **Swap amiibo**: Same as above, but pressing the D-pad left. Moves to the next amiibo in the amiibo directory, if last one starts again with the first one. Only has effect if amiibo emulation is toggled.

Emuiibo's amiibo directory is `sd:/emuiibo`. Place your amiibo dumps (must be `*.bin` files) there.

### Amiibo emulation

Emuiibo gets amiibo's data, but the register info (amiibo name, write dates, mii) is auto-generated, as it isn't present on amiibo dumps:

- Name will be the file's name (`Amiibo.bin` -> `Amiibo`), but if the name is bigger than 10 chars it will be hardcoded to `Emuiibo`.

- Write date is hardcoded to 15th June 2019.

- The amiibo's mii (owner) is hardcoded to the first mii found in the console mii database.

### Amiibo dumps

Dumps consist on `*.bin` files, which must be 540 bytes (perhaps even more?). They can be dumped with several tools.

### For developers

This MitM process also hosts a custom service, `nfp:emu`, which can be used to control amiibo swapping and emulation by IPC.

You have an implementation for C/C++ and libnx in [here](nfpemu-libnx).

## Credits

- All the persons who contributed to the `nfp-mitm` project before me: *Subv, ogniK, averne, spx01, SciresM*

- libstratosphere libraries (SciresM again)