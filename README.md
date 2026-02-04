# Q4WIN10 Window Decoration (Twin)

A lightweight, flat Windows 10 inspired window decoration for Trinity Desktop Environment (TDE).
Designed to match the Q4Win10 Widget Style.

## Features
*   **Windows 10 Aesthetics**: Flat design, authentic button shapes and colors.
*   **lightweight**: ~74KB binary
*   **Performance**: compiled with `-O2 -flto -ffast-math -fmerge-all-constants`.
*   **Hardcoded Configuration**: No external config plugins required.

## Compilation
Requires TDE build environment.

1.  Copy `q4win10` folder to `tdebase/twin/clients/`.
2.  Add to `tdebase/twin/clients/CMakeLists.txt`: `add_subdirectory(q4win10)`.
3.  Build tdebase.

## Packaging
Run `./create_deb.sh` to generate a stand-alone `.deb` package.
Dependencies: `tdebase-trinity`.
