# Bolt Multiplier

A native [OpenRAC](https://github.com/OpenRAC/OpenRAC) mod that brings a **Ratchet & Clank 2-style bolt multiplier** to the original **Ratchet & Clank**.

Build a streak by defeating enemies, earn more bolts, and try to reach **x20** without taking damage. Play with the multiplier from the start, or keep it exclusive to Challenge Mode.

## Features

- **x1–x20 multiplier:** Defeat enemies without taking damage to increase your multiplier. Reaching x20 requires a streak of 100 kills.
- **More bolts:** Collected bolts are multiplied, including bolts dropped by crates. Breaking crates does **not** increase your kill streak.
- **Damage resets the streak:** Losing health or dying resets the multiplier to x1. Streaks also reset when loading a save or changing planets.
- **In-game HUD:** Displays the current multiplier in RAC1's font and fades alongside the normal bolt counter.
- **Two activation modes:** Challenge Mode only (default), or From the start.

## Installation

1. Download the **mod-release** artifact from the [latest successful GitHub Actions build](https://github.com/glawks/bolt-multiplier/actions).
2. Extract the artifact and locate `bolt-multiplier-0.2.4.zip`.
3. Install the mod ZIP using the **Mods** tab in the OpenRAC Launcher.
4. Enable **Bolt Multiplier**, configure its settings, and launch Ratchet & Clank.

## Settings

Find these in **OpenRAC Launcher → Mods → Bolt Multiplier**. Restart the game after changing them.

| Setting | Default | Options |
| --- | --- | --- |
| Activation mode | **Challenge Mode only** | Challenge Mode only / From the start |
| Show multiplier HUD | **On** | On / Off |
| Multiplier HUD placement | **Near bolt (HUD: Edges)** | Near bolt (HUD: Edges) / On bolt icon (HUD: Centre / 4:3) |
| Detailed multiplier log | **Off** | On / Off |

### HUD placement

- **Near bolt:** Places the multiplier near the bolt counter. Intended for OpenRAC's **HUD: Edges** layout.
- **On bolt icon:** Places the multiplier over the silver bolt icon. Intended for **HUD: Centre** or a 4:3 display.

The two placement modes are manual; changing OpenRAC's HUD layout does not automatically switch the mod's setting. The current mod HUD API draws within the original 4:3 area, so it cannot follow the bolt icon all the way to the edge of a widescreen display.

## Compatibility

- **Game:** Ratchet & Clank 1 (PAL and NTSC-U)
- **Platform builds:** Windows x64, Linux x64/arm64, macOS Intel/Apple Silicon
- **OpenRAC mod API:** 1.12 or newer

The gameplay and HUD have been tested in-game on **NTSC-U**. PAL support is included but has not yet been verified in-game.

The mod is currently marked **single-player only**; its bolt economy has not been verified in multiplayer. Mods that change the same game functions may interact depending on load order.

## Building from source

Requires Python 3, CMake 3.20+, Ninja, and a C++17 compiler. The SDK download is pinned in `sdk.lock`.

```bash
python tools/fetch_sdk.py
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

On Windows, use an x64 compiler environment (such as the **x64 Native Tools Command Prompt for Visual Studio**).

The compiled mod is staged under `build/package/bolt_multiplier/`. To package it:

```bash
python .sdk/openrac-mod-sdk-1.14.6/tools/package_mod.py build/package/bolt_multiplier -o dist
```

GitHub Actions also builds and combines all supported platform packages.

## Licence

Licensed under **GPL-3.0-or-later**. Copyright © 2026 glawks.

Enemy-class classification is adapted from the [OpenRAC Difficulty mod](https://github.com/OpenRAC/openrac-mods/tree/master/difficulty-mod). No original game assets are included.
