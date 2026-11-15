# Bolt Multiplier for Ratchet & Clank 1 (OpenRAC)

**Native C++ development version 0.2.5** — a streak-based bolt multiplier for OpenRAC's Ratchet & Clank 1, with a HUD drawn in the game's own font. **Challenge Mode only** is the default, with **From the start** available in the launcher's mod settings. This is a source checkout, not an installable native binary; compile it using the instructions below (or use a GitHub Actions build artifact).

## How it plays

- Begin at **x1**. Killing hostile enemies builds a streak. At 1 kill it becomes x2, at 2 kills x3, and eventually x20 at 100 kills without damage.
- Collecting a bolt gives its normal value, then the mod adds the extra multiple. This includes loose bolts dropped from broken crates, although **crates do not count as kills**.
- When Ratchet actually loses health or dies, the streak returns to **x1**. An invincibility mod that prevents all health loss does not break a streak.
- The streak also resets on a level change or save load. It is not stored in the save file.
- A small `x1`–`x20` indicator uses RAC1's native font and mirrors the bolt counter's fade. Under **HUD: Edges** it appears just below/right of the bolt number (the furthest right the mod HUD API allows). Under **HUD: Centre / 4:3**, choose **On bolt icon** in the mod settings to position the number directly over the silver bolt, using the game's own bolt-HUD record.
- **HUD placement options (v0.2.5):** **Near bolt (HUD: Edges)** is placed below the counter total (not over the number) and leaves room for the entire `x20` label. **On bolt icon (HUD: Centre / 4:3)** is unchanged. **These options don't switch automatically:** OpenRAC 1.12's native HUD text API does not expose whether the player has selected HUD Centre or Edges. If you change the game's HUD layout, match the corresponding mod placement setting and restart. Exact icon anchoring under HUD: Edges requires upstream HUD text anchoring support.
- Works with the supported **RAC1 PAL and NTSC-U** builds. No PS2 assets are included.

## Settings

| Setting | Default | Choices |
| --- | --- | --- |
| Activation mode | **Challenge Mode only** | Challenge Mode only / From the start |
| Show multiplier HUD | **On** | On / Off |
| Multiplier HUD placement | **Near bolt (HUD: Edges)** | Near bolt (HUD: Edges) / On bolt icon (HUD: Centre / 4:3) |
| Detailed multiplier log | **Off** | On / Off |

The settings are in **OpenRAC Launcher → Mods → Bolt Multiplier**. Restart the game after changing them.

## Build (Windows, Linux, macOS)

Install CMake 3.20+, a C++17 compiler, Python 3, and Ninja. The repository pins a known released OpenRAC SDK by SHA-256.

```bash
python tools/fetch_sdk.py
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

The native library and installation folder are generated under `build/package/bolt_multiplier/`. The SDK packages it as an installable ZIP:

```bash
python .sdk/openrac-mod-sdk-1.14.6/tools/package_mod.py build/package/bolt_multiplier -o dist
```

On **Windows**, run the CMake steps in the **x64 Native Tools Command Prompt for VS 2022** (or another environment with an x64 C++ compiler). For source-only rules testing without a downloaded SDK:

```bash
cmake -S . -B build-rules -DBOLT_RULES_ONLY=ON
cmake --build build-rules
ctest --test-dir build-rules --output-on-failure
```

The included GitHub Actions workflow builds platform packages for Windows x64, Linux x64/arm64, and macOS Intel/Apple Silicon, then merges them. Push tag `v0.2.5` **after the tests and in-game verification** to publish a release.

## Multiplier progression

| Kills | Multiplier | Kills | Multiplier |
| ---: | ---: | ---: | ---: |
| 0 | x1 | 30 | x11 |
| 1 | x2 | 36 | x12 |
| 2 | x3 | 42 | x13 |
| 4 | x4 | 49 | x14 |
| 6 | x5 | 56 | x15 |
| 9 | x6 | 64 | x16 |
| 12 | x7 | 72 | x17 |
| 16 | x8 | 81 | x18 |
| 20 | x9 | 90 | x19 |
| 25 | x10 | 100 | x20 |

## Verification checklist

1. Run on **first-playthrough PAL** with default settings: kills do not multiply bolts and the HUD line does not show.
2. Run on **Challenge Mode PAL** with default settings: kills increase x1 → x2 → x3; normal bolt pickup yields the multiplied reward.
3. Switch to **From the start** and reload a new game: multiplier works from the first enemy.
4. Break a crate: collect its bolts at the current multiplier without increasing the streak.
5. Take damage: verify immediate reset to x1. Test death by falling and by crusher too.
6. With game **HUD: Edges**, select mod **Near bolt**: the entire `x1`–`x20` label must sit **below** the bolt number, not overlap it, and must fade with the counter. With game **HUD: Centre**, select **On bolt icon**: the text should stay on the icon. Change the mod setting (and restart the game) when switching game HUD modes; the mod SDK does not currently report this preference.
7. Check an existing Challenge Mode save and a first-playthrough save on **NTSC-U** with **Detailed multiplier log** enabled. **The NTSC-U cycle address is currently inferred**, so this must be verified on real saves before release.
8. Try with other hooks used by your mods (e.g. Difficulty), plus the Cheats mod with its bolt multiplier set to x1. If you enable multiple bolt multipliers, their effects may stack.

**Compatibility:** This test build is marked `multiplayer: solo` to avoid claiming network-synchronised bolt economy before it has been verified. It calls `original()` on all hooked functions to coexist with other single-player mods. An enemy is counted through `Moby_RecordDeath` only if its class is in the hostile-class table, adapted from OpenRAC's Difficulty mod. Special scripted enemy deaths might be missed. HUD text supports ASCII `x`, not a multiplication sign (`×`). Extra bolts are credited after the original pickup; some historical per-level pickup statistics may retain the base amount.

**Current verification status:** The standalone C++ streak and bonus calculations have been built and tested. **The native shared library has not yet been compiled against the downloaded SDK or tested in-game**, so this is not a release-ready binary.

## Source and licence

Mod source licensed GPL-3.0-or-later; copyright © 2026 glawks. Enemy classification is based on [OpenRAC Difficulty mod](https://github.com/OpenRAC/openrac-mods/tree/master/difficulty-mod), also GPL-3.0-or-later. OpenRAC's modding SDK is MIT licensed. No original game files are redistributed.
