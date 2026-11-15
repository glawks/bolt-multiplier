# Native HUD placement diagnostic

This branch is an experimental placement probe, not a release candidate.
It uses the supported `rac1.hud.quick_select.draw` hook, calls its original,
then draws a fixed `x20` using RAC1's font texture, glyph metrics and native
textured-quad function. It does not change the streak or bolt-award rules.

OpenRAC temporarily moves all side-anchored HUD slots while traversing the
HUD. The probe reads the bolt slot **inside** that traversal, so its x already
includes the Centre/Edges displacement. It bypasses `hud_text_set()` and its
0–1 coordinate validation. No renderer patch or HUD callback replacement is
used. PAL and NTSC addresses follow OpenRAC's version maps; resolve failure
logs an error and skips drawing.

## Run the test

1. Download the `mod-windows-x86_64` artifact from this branch's build action
   (or the combined `mod-release` artifact once every platform finishes).
2. Install this test in place of the stable Bolt Multiplier. It has the same
   mod id, so install one version at a time. Keep the stable archive to revert.
3. Leave **Native HUD drawing test** and **Show multiplier HUD** enabled.
   Restart the game after changing settings.
4. Load a planet and open/hold Quick Select. A fixed `x20` should appear
   beneath the bolt icon's position. No kills or Challenge Mode are required.
5. Compare 4:3, 16:9 Centre, and 16:9 Edges; try ultrawide if available.
   The text should stay beneath the bolt icon position and move right with
   Edges, beyond the central 4:3 region. Check PAL and NTSC separately.
6. Close Quick Select, pause/unpause, and change planets. The diagnostic
   should only draw when the game calls the Quick Select draw callback.

The normal multiplier text is suppressed while the test is enabled. The
fixed label deliberately ignores the bolt counter's fade and activation mode
so visibility cannot hide the placement result. Disable the diagnostic and
restart to return to normal HUD text.

## Report back

Send a screenshot with Quick Select open in 16:9 Edges, your disc version,
and log lines containing `Native HUD probe`. The log records the bolt slot's
x and the text's centre whenever the level or position changes. If no probe
line appears after opening Quick Select, the drawing hook did not reach the
probe or the bolt slot definition was unavailable; include the full log.

This only tests native placement during Quick Select. The SDK does not
currently expose the bolt counter's draw callback as a hook point, so this
does **not** provide an always-visible native multiplier. A successful test
establishes the drawing/coordinate path; selecting a reliable callback for
normal gameplay is separate work.

## Local validation

Build against the pinned SDK with `python tools/fetch_sdk.py`,
`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`, `cmake --build build`, and
`ctest --test-dir build --output-on-failure`. Compilation and core tests do
not establish runtime drawing correctness; the in-game checks above do.
