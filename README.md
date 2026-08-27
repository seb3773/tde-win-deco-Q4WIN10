# Q4WIN10 Window Decoration for Trinity Desktop (TDE)

A lightweight, modern Windows 10 & Windows 11 inspired window decoration plugin for the Trinity Desktop Environment (TDE / Twin), based on Plastik and designed to match the Q4OS Q4Win10 theme style.

Compatible across **all Trinity Desktop R14.1.x versions** (R14.1.0 to R14.1.6+) as a single standalone universal binary.

---

## ✨ Features & Highlights

* **Windows 11 Mode**:
  * Floating rounded titlebar buttons matching modern Windows 11 aesthetics.
  * Rounded window corners (8px radius) with smooth clipping.
  * Subtle hover outlines and highlights.
* **Dark Mode Feature**:
  * Toggleable high-contrast white button icons, designed for dark desktop themes.
* **Context-Aware Borders & Menubar Integration**:
  * Seamless X11 Atom integration with the widget style (`_Q4WIN10_MENUBAR_HEIGHT`) to match the menubar color seamlessly with zero per-frame X11 round-trip overhead.
* **Interactive Configuration Module**:
  * Dedicated graphical configuration dialog in Trinity Control Center.
  * Embedded vector-clean branding logo (128x74 smoothed at runtime) and build version information.
  * Dynamic toggle of Dark Mode and Windows 11 Mode without restarting Twin.
* **Ultra-Aggressive Optimization & Memory Safety**:
  * Compiled with `-O2 -flto=auto -ffunction-sections -fdata-sections -fno-exceptions -fomit-frame-pointer -fvisibility=hidden`.
  * Zero memory leaks, guarded edge-case dimensions, and reusable pixmap double-buffering.
  * Super-stripped (`sstrip`):
    * **Decoration Plugin (`twin3_q4win10.so`)**: **~68 KB**
    * **Config Plugin (`twin_q4win10_config.so`)**: **~31 KB**
    * **Debian Package (`.deb`)**: **~25 KB**
    * **Q4OS Installer (`.qsi`)**: **~86 KB**

---

## 🚀 Easy Installation

### 1. Q4OS Graphical Installer (`.qsi`) — Recommended for Q4OS
Download or build the `.qsi` installer:
* **Double-click** `setup_tde-win-deco-q4win10_2.0.1.qsi` in the file manager, or run:
```bash
./setup_tde-win-deco-q4win10_2.0.1.qsi
```
Follow the graphical setup wizard to install and automatically register the decoration.

### 2. Debian Package (`.deb`) — Any Debian / Ubuntu / TDE system
Install the universal package via `dpkg`:
```bash
sudo dpkg -i tde-win-deco-q4win10_2.0.1_amd64.deb
```

### 3. Activation
Once installed, update the sycoca cache if needed:
```bash
tdebuildsycoca
```
Open **Trinity Control Center → Appearance & Themes → Window Decorations**, select **Q4WIN10**, and click **Apply**.

---

## 🛠️ Building from Source

### Prerequisites
Ensure standard TDE and TQt development packages are installed:
```bash
sudo apt install tde-cmake-trinity tdebase-trinity-dev tdelibs14-trinity-dev libtqt3-mt-dev tqt3-dev-tools sstrip
```

### Method A: Build Universal `.deb` and `.qsi` Packages
To generate both distribution packages with automated versioning:
```bash
# Build .deb package:
./create_deb.sh

# Or build both .deb and .qsi installer:
./create_qsi.sh
```

### Method B: Standard CMake Build
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/trinity -DQ4WIN10_AGGRESSIVE_FLAGS=ON
make -j$(nproc)
sudo make install
```

### Method C: Direct Standalone Makefile (Fast Build)
```bash
make clean && make
sudo make install
```

---

## 🖼️ Screenshots

![buttons deco](screenshots/win10deco_screen1.png) ![buttons decos](screenshots/win10deco_screen2.png) ![buttons deco](screenshots/win10deco_screen3.png)
![buttons deco](screenshots/win10deco_screen4.png) ![buttons deco](screenshots/win10deco_screen5.png)

---

## 📄 License & Credits

* **Author**: Seb3773 (<https://github.com/seb3773>)
* **Based on**: Plastik KWin decoration by Sandro Giessl and Web decoration by Rik Hemsley
* **License**: GNU General Public License v2 (GPL-2.0+)