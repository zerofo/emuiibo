<p align="center">
  <img alt="Discord Server" src="emutool/PcIcon.png">
</p>
<h1 align="center">
  emuiibo
</h1>

<p align="center">
  Virtual amiibo (amiibo emulation) system for Nintendo Switch
</p>

<p align="center">
  <a title="Discord" href="https://discord.gg/3KpFyaH">
    <img alt="Discord" src="https://img.shields.io/discord/789833418631675954?label=Discord&logo=Discord&logoColor=fff&style=for-the-badge">
  </a>
  <a title="Downloads" href="https://github.com/XorTroll/emuiibo/releases/latest">
    <img alt="Downloads" src="https://img.shields.io/github/downloads/XorTroll/emuiibo/total?longCache=true&style=for-the-badge&label=Downloads&logoColor=fff&logo=GitHub">
  </a>
  <a title="License" href="https://github.com/XorTroll/emuiibo/blob/master/LICENSE">
    <img alt="License" src="https://img.shields.io/github/license/XorTroll/emuiibo?style=for-the-badge">
  </a>
</p>

<p align="center">
  <a title="b" href="https://www.patreon.com/xortroll">
    <img alt="Patreon" src="https://img.shields.io/endpoint.svg?url=https%3A%2F%2Fshieldsio-patreon.vercel.app%2Fapi%3Fusername%3DXorTroll%26type%3Dpatrons&style=for-the-badge"/>
  </a>
  <a href="https://github.com/sponsors/XorTroll">
    <img alt="GitHub sponsors" src="https://img.shields.io/github/sponsors/XorTroll?label=Sponsor&logo=GitHub&style=for-the-badge"/>
  </a>
  <a href="https://www.paypal.com/donate/?hosted_button_id=PHQKFTY9AHPUU">
    <img alt="PayPal" src="https://img.shields.io/badge/Donate-PayPal-green.svg?style=for-the-badge"/>
  </a>
</p>

<h3>
Table of contents
</h3>

- [Virtual amiibos](#virtual-amiibos)
  - [Virtual amiibo creation](#virtual-amiibo-creation)
  - [Supported figures](#supported-figures)
  - [Areas](#areas)
  - [Per-game access IDs](#per-game-access-ids)
- [Controlling emuiibo](#controlling-emuiibo)
- [Boot procedure](#boot-procedure)
- [For developers](#for-developers)
- [Credits](#credits)
  - [License exemption](#license-exemption)

## Virtual amiibos

Virtual amiibos go inside `sd:/emuiibo/amiibo`. For instance, an amiibo named `MyMario` would be `sd:/emuiibo/amiibo/MyMario/<amiibo content>`. They can go inside sub-directories, like `sd:/emuiibo/amiibo/SSBU/Yoshi`.

A virtual amiibo is detected by emuiibo based on two aspects: a `amiibo.json` and a `amiibo.flag` file must exist inside the virtual amiibo's folder mentioned above. If (for whatever reason) you would like to disable a virtual amiibo from being recognised by emuiibo, just remove the flag file, and create it again to enable it.

The JSON file contains all the aspects and data an amiibo needs to provide to games, except a few special ones (per-game savedata, protocol and tag type...)

This are the properties an amiibo has:

- Name: the amiibo's name (max. 10 characters, longer names will be automatically limited to 10 by emuiibo)

- UUID: it's a unique identifier for the amiibo, composed of 10 bytes. Common amiibo UUID values seem to have the last 3 bytes zeroed...?

- Random UUID: if this option is enabled (set to true), emuiibo will ignore the fixed UUID value and generate random UUIDs every time the amiibo is accessed by a game. This has potential benefits in certain games, like in BOTW, where amiibos can only be used once per day, but with randomized UUIDs this can be bypassed, and one can get infinite rewards scanning this amiibo infinite times.

- Mii: every amiibo has a mii associated with it (it's "owner"). Internally, miis consist on a 88-byte structure known as "char-info", so emuiibo stores this data in a file (typically `mii-charinfo.bin`). For new amiibos, emuiibo uses the console's services to generate a random mii, but for those who would like to use a mii from their console, emuiibo dumps in `miis` directory the console's miis, so it's just a matter of copying and pasting/replacing charinfo bin files. *NOTE*: emuiibo contains the charinfo file's name in the JSON (`mii_charinfo_file`), so if the file ever gets renamed, don't forget to rename it in the JSON too, or emuiibo will generate a random mii for the file name in the JSON.

- First and last write dates: these are (as if it wasn't obvious) the first and last time the amiibo was written/modified. When a virtual amiibo is created with emutool, the current date is assigned to both dates, and when the amiibo is modified in console, emuiibo updates the last write date.

- Write counter: this is a number which is increased everytime the amiibo is modified (and emuiibo does so, imitating Nintendo), but when the number reaches 65535, it is no longer increased (the number is technically a 16-bit number)

- Version: this value technically represents the version of Nintendo's amiibo library (NFP), so emuiibo just defaults it to 0.

### Virtual amiibo creation

While old emuiibo formats are supported and converted to the current format (see above), it is strongly suggested to, unless bin dumps might be indispensable, `emutool` be used, our PC tool designed to create virtual amiibos:

![Screenshot](emutool/Screenshot.png)

### Supported figures

Some games (like Skylanders) make use of their particular NFC technology, aside from amiibos. This project ONLY emulates amiibos; therefore, in Skylanders' case, only the two special figurines with amiibo support can be emulated here, where emuiibo will only emulate the "amiibo part" of them.

### Areas

Areas (application areas, technically) are per-game amiibo savedata. Technically, real amiibos can only save data for a single game, but emuiibo allows as many games as you want (since savedata is stored as files). This savedata is quite small, and tends to be 216 bytes or smaller.

emuiibo saves this data inside bin files at `areas` directory inside the amiibo's directory, and the bin file's name is the game's area access ID in hex format.

An access ID is a unique ID/number each game has for amiibo savedata, used to check if the game actually has savedata in an amiibo. Here's a list of games and their access IDs:

### Per-game access IDs

| Game                                    | Application ID   | Access ID  |
|-----------------------------------------|------------------|------------|
| Splatoon 2                              | 0x01003BC0000A0000 | 0x10162B00 |
| Shovel Knight: Treasure Trove           | 0x010057D0021E8000 | 0x1016E100 |
| The Legend of Zelda: Breath of the Wild | 0x01007EF00011E000 | 0x1019C800 |
| Super Smash Bros. Ultimate              | 0x01006A800016E000 | 0x34F80200 |
| Splatoon 3                              | 0x0100C2500FC20000 | 0x38600500 |
| The Legend of Zelda: Link's Awakening   | 0x01006BB00C6F0000 | 0x3B440400 |

Some of these IDs were obtained from [switchbrew](https://switchbrew.org/w/index.php?title=NFC_services#Access_ID).

## Controlling emuiibo

- **Emulation status (on/off)**: when emuiibo's emulation status is on, it means that any game trying to access/read amiibos will be intercepted by emuiibo. When it's off, it means that amiibo services will work normally, and nothing will be intercepted. This is basically a toggle to globally disable or enable amiibo emulation.

- **Active virtual amiibo**: it's the amiibo which will be sent to the games which try to scan amiibos, if emulation is on. Via tools such as the overlay, one can change the active virtual amiibo.

- **Virtual amiibo status (connected/disconnected)**: when the active virtual amiibo is connected, it means that the amiibo is always "placed", as if you were holding a real amiibo on the NFC point and never moving it - the game always detects it. When it is disconnected, it means that you "removed" it, as if you just removed the amiibo from the NFC point. Some games might ask you to remove the amiibo after saving data, so you must disconnect the virtual amiibo to "simulate" that removal. This is a new feature in v0.5, which fixed errors, since emuiibo tried to handle this automatically in previous versions, causing some games to fail.

All this aspects (and more) can be seen/controlled via the overlay.

## Boot procedure

- This is the list of actions emuiibo will automatically do when booting the console:

  - Console miis are saved at `sd:/emuiibo/miis/<index>`, both their name (in a text file since it might not be ASCII) and their data as a `mii-charinfo.bin` file.

  - Old amiibo formats (raw bin dumps and old virtual amiibo formats) are automatically converted to the modern format:

    - If `sd:/switch/key_retail.bin` is present, the bin dump's encrypted sections can be also used, which means that amiibo settings (mii, name, etc.) and game app-area (save-data), if present, are also extracted for the new format.

    - The bin dump file does not get removed, but it will get moved inside the modern amiibo format, along with the decrypted file.

## For developers

emuiibo hosts a custom IPC service, also named `emuiibo`, which can be used to control amiibo emulation by other homebrew tools.

The overlay's code serves a good example to see how to control emuiibo with libnx.

## Credits

- Everyone who contributed to the original **nfp-mitm** project (and other forks): *Subv, ogniK, averne, spx01, SciresM*

- **AmiiboAPI** web API, which is used by `emutool` to get a proper, full amiibo list, in order to generate virtual amiibos.

- [**3dbrew**](https://www.3dbrew.org/wiki/Amiibo) for their detailed documentation of amiibos, even though some aspects are different on the Switch.

- **LoOkYe** for writing emuiibo's wiki and helping with support.

- **AD2076** and **AmonRaNet** for helping with the tesla overlay.

- **AmonRaNet** for all the work he put into the overlay.

- **AmonRaNet**, **Impeeza**, **amazingfate**, **qazrfv1234**, **shadow2560** and **SimoCasa** for providing/helping with translations.

- **Thog** / **Ryujinx** devs for reversing mii services and various of its types.

- **Citra** devs for several amiibo formats used in 3DS systems.

- **Manlibear** for helping with improvements and development of `emutool`.

- All the testers and supporters from my Discord server who were essential for making this project progress and become what it is now :)

### License exemption

- The Ryujinx project/team is exempt from GPLv2 licensing, and may make use of emuiibo's code licensing it under their current license.