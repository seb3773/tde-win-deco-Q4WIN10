# Q4WIN10 Window Decoration (Twin)

A lightweight, flat Windows 10 inspired window decoration for Trinity Desktop Environment (TDE).
Designed to match the Q4Win10 Widget Style.

## Features
*   **Windows 10 Aesthetics**: Flat design, authentic button shapes and colors.
*   **Configuration Support**: Includes a dedicated configuration panel with "Dark Mode" support.
*   **Highly Optimized**: Compiled with `-O2 -flto -ffast-math -fmerge-all-constants`.
*   **Minimized Binary**: Uses `sstrip` to reach ~20KB-40KB per module.


## Integration & Compilation

There are two ways to build Q4WIN10:

### 1. Integrated Build (tdebase)
Recommended if you are already compiling `tdebase-trinity`.

1.  Copy the `tde-win-deco-Q4WIN10` folder to `tdebase/twin/clients/`.
2.  Add to `tdebase/twin/clients/CMakeLists.txt`:
    ```cmake
    add_subdirectory(tde-win-deco-Q4WIN10)
    ```
3.  Re-run your TDE build process (CMake/Ninja/Make).

### 2. Standalone Build (Advanced / Custom TDE)
Use this if you have a pre-installed TDE environment or want to build only the decoration.

1.  Navigate to the source folder:
    ```bash
    cd twin/clients/tde-win-deco-Q4WIN10
    ```
2.  **Adjust Paths (Optional)**: Open `Makefile` and `config/Makefile`. 
    - `TDE_PREFIX` (default: `/opt/trinity`)
    - `TQT_INCLUDE` (default: `/usr/include/tqt3`)
3.  **Compile & Install**:
    ```bash
    make && sudo make install
    ```
    This builds both the decoration and the config module and installs them to `/opt/trinity/lib/trinity/`.

## Debian Packaging

To create a standalone Debian package (`.deb`) ready for distribution:
1.  Ensure you have `dpkg-dev` installed.
2.  Run the packaging script:
    ```bash
    ./create_deb.sh
    ```
The resulting package will be named `tde-win-deco-q4win10_<version>_tde14.1.1_<arch>.deb`.

## Important: Plugin Loading (.la files)

Trinity's `KLibLoader` strictly requires `.la` files in `/opt/trinity/lib/trinity/`.
- If you change the installation path (`TDE_PREFIX`), you **must** update the `libdir` line inside `twin3_q4win10.la` and `twin_q4win10_config.la`.
- Without matching `.la` files, the decoration will fail to load and TDE will fallback to **Plastik**.

## Optimization Notes
The standalone Makefile uses `sstrip` (Super-Strip) to minimize binary size.
- Standard `.so`: ~35-40 KB
- Super-stripped `.so`: ~16-18 KB (optimized for R/O memory)

## Packaging
Run `./create_deb.sh` to generate a stand-alone `.deb` package.
Dependencies: `tdebase-trinity`.
