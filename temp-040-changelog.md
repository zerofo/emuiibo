# Changelog

## Virtual amiibo emulation

- emuiibo was updated to latest libnx and libstratosphere, what might fix possible bugs those had before. (libstratosphere's new IPC API is far more accurate now)

- Internal heap usage was decreased from 0x75000 to 0x5000. (almost 24 times less heap!)

- Several implementation mistakes were corrected, which might have caused issues.

- Those problems with certain amiibos (mainly BOTW ones) have been fixed, which were caused by a bad handling of amiibo IDs (N sends them in a slightly different way)

- Now emuiibo has a settings file: `settings.json` inside emuiibo's directory. The only setting it has for now is whether input combos are enabled, and this defaults to true.

- If you want to disable input combos, just edit the file and change the field `comboEnabled` from `true` to `false`, or the opposite to re-enable them.

- Added logging, so that most interactions are logged to a logging file (`emuiibo.log`) in emuiibo's directory, to help with potential issues.

## Custom service (nfp:emu)

- API changed, some commands' implementation is slightly different now.

- Version now isn't 3 u32, it's a struct made of 4 u8s (major, minor micro and dev - whether it's a dev build).

## emutool (emuGUIibo was renamed to emutool)

- Small corrections in texts.