# Changelog

## Virtual amiibo emulation

- emuiibo was updated to latest libnx and libstratosphere, what might fix possible bugs those had before. (libstratosphere's new IPC API is far more accurate now)

- Internal heap usage was decreased from 0x75000 to 0x40000.

- Several implementation mistakes were corrected, which might have caused issues.

- Those problems with certain amiibos (mainly BOTW ones) have been fixed, which were caused by a bad handling of amiibo IDs (N sends them in a slightly different way)

## Custom service (nfp:emu)

- API changed, some commands' implementation is slightly different now.

- Version now isn't 3 u32, it's a struct made of 4 u8s (major, minor micro and dev / whether it's a dev build).

## emutool (emuGUIibo was renamed to emutool)

- Small corrections in texts.