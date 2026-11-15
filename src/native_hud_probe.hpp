// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 glawks
#pragma once

#include "openrac/mod.hpp"
#include <cstdint>
#include <string>

namespace native_hud_probe {

// Diagnostic only: called AFTER the real Quick Select draw, inside the native
// HUD traversal. No callback pointers, HUD slot positions, or renderer state
// are changed. Resolve functions/data again on every draw, never across loads.
inline void draw(openrac::Mod& mod, bool pal, uint32_t hud_slots) {
    const int level = mod.loaded_level();
    if (level < 0) return;
    const auto memory = mod.memory();
    const auto slot = mod.relocate(hud_slots) + 2u * 0x90u;
    if (memory.read<int32_t>(slot + 0x20u) != 0x754E) return;

    using Texture = unsigned long long (*)(int32_t);
    using QuadPal = void (*)(int32_t, int32_t, int32_t, int32_t,
                            int32_t, int32_t, int32_t, int32_t,
                            unsigned long long, unsigned long long);
    using QuadNtsc = void (*)(int32_t, int32_t, int32_t, int32_t,
                             int32_t, int32_t, int32_t, int32_t,
                             long long, long long);
    // Addresses/types from OpenRAC's rac1/host/hud.c, ui2d.c and the PAL/NTSC
    // rac1_version.h maps. function_at asks the port for native host code;
    // never reinterpret a PS2 memory address as executable host code.
    const auto texture_fn = mod.function_at<Texture>(pal ? 0x001F4868u : 0x001F44B8u);
    const auto quad_pal = pal ? mod.function_at<QuadPal>(0x001F5800u) : nullptr;
    const auto quad_ntsc = pal ? nullptr : mod.function_at<QuadNtsc>(0x001F5450u);
    static bool warned = false;
    if (!texture_fn || (!quad_pal && !quad_ntsc)) {
        if (!warned) mod.error("Native HUD probe: texture/quad function missing; no draw attempted");
        warned = true;
        return;
    }
    const auto table = mod.relocate(pal ? 0x001DF770u : 0x001DF3F0u);
    const auto texture = texture_fn(2);
    constexpr char label[] = "x20"; // fixed, widest normal multiplier; no streak required
    constexpr float scale = 0.8f;
    constexpr int size = 13; // round(16 * scale)
    int width = 0;
    for (const char c : std::string(label)) {
        width += static_cast<int>(memory.read<int8_t>(table + 4u * c + 3u) * scale + 0.5f);
    }
    const int slot_x = memory.read<int32_t>(slot + 0x50u);
    const int centre_x = slot_x - 16;
    const int top_y = (pal ? 10 : 18) + 36;
    // At this point slot_x already includes HUD: Edges. Do not clamp to 512,
    // and do not add the widescreen offset a second time.
    for (int pass = 0; pass < 2; ++pass) {
        const int offset = pass == 0 ? 1 : 0;
        const unsigned long long colour = pass == 0 ? 0x80000000ull : 0x80808080ull;
        int x = centre_x - width / 2 + offset;
        for (const char c : std::string(label)) {
            const auto glyph = table + 4u * c;
            const int advance = memory.read<int8_t>(glyph + 3u);
            if (advance == 0) continue;
            const int y = top_y + static_cast<int>(memory.read<int8_t>(glyph + 2u) * scale) + offset;
            const int u = memory.read<uint8_t>(glyph);
            const int v = memory.read<uint8_t>(glyph + 1u);
            if (quad_pal) quad_pal(x, y, size, size, u, v, 16, 16, colour, texture);
            else quad_ntsc(x, y, size, size, u, v, 16, 16,
                           static_cast<long long>(colour), static_cast<long long>(texture));
            x += static_cast<int>(advance * scale + 0.5f);
        }
    }
    static int last_level = -2;
    static int last_x = -99999;
    if (last_level != level || last_x != slot_x) {
        mod.info("Native HUD probe: level=" + std::to_string(level)
                 + ", bolt slot x=" + std::to_string(slot_x)
                 + ", text centre x=" + std::to_string(centre_x));
        last_level = level;
        last_x = slot_x;
    }
}
} // namespace native_hud_probe
