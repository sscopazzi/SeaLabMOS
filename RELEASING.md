# Releasing firmware

The compiled SeaLabMOS firmware (`.uf2`, `.bin`) has two LGPL libraries built into it: the Arduino-Pico
core (LGPL-2.1-or-later) and Adafruit NeoPixel (LGPL-3.0-or-later). The LGPL lets you hand out the compiled
image only if the source of those libraries, at the versions you built with, can be had from the same
place. For this repository, that place is a GitHub release. The firmware's own source, under the GPL, has to
be there as well.

Whenever a compiled image is published, whether a new one in `SeaLabMOS_master_program/build/` or one
attached to a release, make a GitHub release for that same commit.

## What goes in the release

1. **Tag**: the commit that holds the image and the source it was built from, for example
   `firmware-2026-11-26`. GitHub adds that commit's "Source code" archives to the release by itself. They are
   the firmware's complete source, with `LICENSES/` (GPL, LGPL-2.1 and LGPL-3.0 texts) and
   `THIRD_PARTY_NOTICES.md`.
2. **The image**: `SeaLabMOS_master_program.ino.uf2` (and `.bin` if you publish it).
3. **The Arduino-Pico core, at the version used**: the board package from the core's own release page,
   unchanged. For 5.5.0 that is `rp2040-5.5.0.zip` (110 MB):
   https://github.com/earlephilhower/arduino-pico/releases/download/5.5.0/rp2040-5.5.0.zip
   It is the complete source of the core, its Wire and SPI libraries and ArduinoCore-API, and their licences.
4. **Adafruit NeoPixel, at the version used**: the source archive of that tag. Read the version in the
   Arduino IDE (Library Manager, Installed), then attach
   `https://github.com/adafruit/Adafruit_NeoPixel/archive/refs/tags/<version>.zip`, renamed
   `Adafruit_NeoPixel-<version>.zip`.
5. **Release notes**: state the following.
   - The board package version and every library version the image was built with.
   - That the source of the LGPL libraries is attached, and that the firmware can be rebuilt (with changed
     libraries if you like) in the Arduino IDE from the source archive. Give the board and library versions.
   - How to install a rebuilt image on the Feather RP2040 Adalogger: hold BOOT, tap RESET, and copy the
     `.uf2` onto the RPI-RP2 drive that appears.

Keep the release, and its attached sources, up for as long as the image can be downloaded from this
repository.

## Finding the versions

- **Board package**: Tools > Board > Boards Manager, "Raspberry Pi Pico/RP2040/RP2350". The linker map in
  `build/` shows it too, in its paths (`.../packages/rp2040/hardware/rp2040/5.5.0/...`).
- **Libraries**: Library Manager, Installed. List each one that `THIRD_PARTY_NOTICES.md` names under
  "Compiled into the published firmware".

## The first release

Build the image with Arduino-Pico 5.5.0 and Adafruit NeoPixel 1.15.4 (Sketch > Export Compiled Binary), commit it
in `SeaLabMOS_master_program/build/` with the rest of the change, and make the release from that commit. Attach
`rp2040-5.5.0.zip` and `Adafruit_NeoPixel-1.15.4.zip`.
