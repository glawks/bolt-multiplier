// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 glawks
// Bolt Multiplier for OpenRAC's native Ratchet & Clank 1 port.
// Challenge Mode only is the default; optional activation from the start.
// The game and earlier mods keep their original hooks. No renderer patching:
// OpenRAC API 1.12 draws the indicator in RAC1's own HUD font.
#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"
#include "rules.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace {
namespace rac1 = openrac::rac1;
namespace rules = bolt_multiplier;

// Game globals, PAL SCES-50916 and NTSC-U SCUS-97199. All reads go through
// relocate_data, matching the shipped SDK's address relocation convention.
struct Disc {
    uint32_t hero;
    uint32_t challenge_cycle;
    uint32_t hud_slots;
};
constexpr Disc kPal{0x0013F450u, 0x0015EF20u, 0x00199C60u};
// NTSC's cycle pointer follows the -0x100 offset seen in related globals.
// Its exact position must still be checked against an NTSC Challenge Mode save.
constexpr Disc kNtsc{0x0013F350u, 0x0015EE20u, 0x00199B60u};
constexpr uint32_t kHealthOffset = 0x22A8u;
constexpr uint32_t kMobyClassOffset = 0xA6u;
constexpr uint32_t kSafeMaxCycle = 999u;

openrac::Mod* s_mod = nullptr;
const Disc* s_disc = nullptr;
rules::Streak s_streak;
std::string s_mode = "challenge";
bool s_show_hud = true;
bool s_log_changes = false;
bool s_last_active = false;
bool s_have_cycle = false;
uint32_t s_last_cycle = 0;
int s_last_level = -2;
int s_hud_level = -2;
int s_hud_multiplier = -1;
bool s_hud_active = false;
bool s_hud_visible = false;
int s_hud_alpha = -1;
int32_t s_last_health = 0;
bool s_have_health = false;
std::unordered_set<uint32_t> s_deaths_this_tick;

uint32_t current_cycle() {
    if (!s_mod || !s_disc) return 0;
    const uint32_t cycle = s_mod->memory().read<uint32_t>(s_mod->relocate(s_disc->challenge_cycle));
    return cycle <= kSafeMaxCycle ? cycle : 0;
}

int32_t health() {
    if (!s_mod || !s_disc) return 0;
    return s_mod->memory().read<int32_t>(s_mod->relocate(s_disc->hero) + kHealthOffset);
}

bool active(uint32_t cycle) { return s_mode == "always" || cycle > 0; }

void log_change(const std::string& message) {
    if (s_log_changes) s_mod->info(message);
}

void invalidate_hud() {
    s_hud_level = -2;
    s_hud_multiplier = -1;
    s_hud_active = false;
    s_hud_visible = false;
    s_hud_alpha = -1;
}

void reset_streak(const char* reason) {
    if (s_streak.kills() != 0) {
        log_change(std::string("x1 (streak reset: ") + reason + ")");
    }
    s_streak.reset();
    s_hud_multiplier = -1;
}

void reset_for_save(const char* reason) {
    reset_streak(reason);
    s_have_cycle = false;
    s_have_health = false;
    s_deaths_this_tick.clear();
    invalidate_hud();
}

void synchronize_cycle() {
    const uint32_t cycle = current_cycle();
    const bool now_active = active(cycle);
    if (!s_have_cycle) {
        s_last_cycle = cycle;
        s_last_active = now_active;
        s_have_cycle = true;
        log_change("activation=" + s_mode + ", NG+ cycle=" + std::to_string(cycle)
                   + (now_active ? ", active" : ", inactive"));
    } else if (s_last_cycle != cycle || s_last_active != now_active) {
        reset_streak("playthrough changed");
        log_change("NG+ cycle=" + std::to_string(cycle)
                   + (now_active ? ", active" : ", inactive"));
        s_last_cycle = cycle;
        s_last_active = now_active;
    }
}

// OpenRAC's HUD text does not inherit the bolt counter's animation.
// RAC1's counter is HUD slot 2 (definition 0x754E at +0x20); the original
// game updates its panel and digit fade at +0x70/+0x71. Field +0x64 is a
// generated serial/handle, NOT the definition id. These offsets are common
// to PAL/NTSC, as confirmed by the game's queue_animation_update().
int bolt_counter_alpha() {
    constexpr uint32_t kSlotBytes = 0x90u;
    const auto slot = s_mod->relocate(s_disc->hud_slots) + 2u * kSlotBytes;
    const auto memory = s_mod->memory();
    if (memory.read<int32_t>(slot + 0x20u) != 0x754E) return 0;
    const uint8_t panel = memory.read<uint8_t>(slot + 0x70u);
    const uint8_t fade = memory.read<uint8_t>(slot + 0x71u);
    if (panel == 0 || fade == 0) return 0;
    const unsigned alpha = static_cast<unsigned>(fade) * 32u;
    return static_cast<int>(alpha > 255u ? 255u : alpha);
}

// The game's font is supported by the SDK 1.12 HUD text API, but the
// coordinates are limited to the original 4:3 canvas. HUD: Edges shifts
// the game's bolt panel farther right in widescreen; put our label as
// close to it as this SDK currently permits.
void update_hud(int level) {
    if (level < 0) {
        invalidate_hud();
        return;
    }
    const bool on = active(current_cycle());
    const int alpha = on && s_show_hud ? bolt_counter_alpha() : 0;
    const bool visible = alpha != 0;
    const int multiplier = s_streak.multiplier();
    if (s_hud_level == level && s_hud_multiplier == multiplier
        && s_hud_active == on && s_hud_visible == visible && s_hud_alpha == alpha) {
        return;
    }
    std::vector<openrac_hud_text> lines;
    if (visible) {
        openrac_hud_text text{};
        std::snprintf(text.text, sizeof text.text, "x%d", multiplier);
        text.x = 0.985f;
        text.y = 0.132f;
        text.scale = 0.80f;
        text.rgba[0] = 128;
        text.rgba[1] = 128;
        text.rgba[2] = 128;
        text.rgba[3] = static_cast<uint8_t>(alpha);
        text.flags = OPENRAC_HUD_SMALL | OPENRAC_HUD_SHADOW;
        lines.push_back(text);
    }
    // The port refuses text during level load, so retry next tick.
    if (!s_mod->hud_text_set(lines)) return;
    s_hud_level = level;
    s_hud_multiplier = multiplier;
    s_hud_active = on;
    s_hud_visible = visible;
    s_hud_alpha = alpha;
}

void frame() {
    // At the frame boundary we may accept the same moby address again after
    // the pool has spawned a new enemy. Multiple death callbacks in one tick
    // only count once per moby address.
    s_deaths_this_tick.clear();
    const int level = s_mod->loaded_level();
    if (level < 0) {
        s_last_level = -2;
        s_have_health = false;
        invalidate_hud();
        return;
    }
    if (s_last_level != level) {
        reset_streak("level changed");
        s_last_level = level;
        s_have_health = false;
        invalidate_hud();
    }
    synchronize_cycle();
    const int32_t hp = health();
    // Ordinary hits are handled by the hook. This also covers falls,
    // crushers, and scripted health loss that bypass Hero_TakeDamage.
    if (active(current_cycle()) && s_have_health && hp < s_last_health) {
        reset_streak("health decreased");
    }
    if (active(current_cycle()) && hp <= 0 && s_streak.kills() > 0) {
        reset_streak("Ratchet died");
    }
    s_have_health = true;
    s_last_health = hp;
    update_hud(level);
}

void load(openrac::Mod& mod) {
    if (mod.game() != "rac1") {
        throw openrac::Error("Bolt Multiplier supports Ratchet & Clank 1 only");
    }
    if (mod.version() == "pal") s_disc = &kPal;
    else if (mod.version() == "ntsc") s_disc = &kNtsc;
    else throw openrac::Error("Bolt Multiplier supports RAC1 PAL and NTSC-U only");

    s_mod = &mod;
    s_mode = mod.setting<std::string>("activation_mode");
    if (s_mode != "challenge" && s_mode != "always") s_mode = "challenge";
    s_show_hud = mod.setting<bool>("show_multiplier");
    s_log_changes = mod.setting<bool>("debug_log");
    reset_for_save("mod loaded");
    s_last_level = -2;

    // The game calls NewGameInit both for a normal new game and *inside* the
    // transition to Challenge Mode. The later NewGamePlus call re-reads the
    // persistent cycle, which is incremented by the game itself.
    mod.hook<rac1::Save_NewGame>([](auto original) {
        original();
        reset_for_save("new game");
    });
    mod.hook<rac1::Save_NewGamePlus>([](auto original, int32_t slot) {
        original(slot);
        reset_for_save("Challenge Mode started");
    });
    mod.hook<rac1::Save_Restore>([](auto original, uint32_t save) {
        original(save);
        reset_for_save("save restored");
    });
    mod.hook<rac1::Save_RestoreData>([](auto original, uint32_t source, int32_t slot,
                                       uint32_t table) -> int32_t {
        const int32_t result = original(source, slot, table);
        if (result != 0) reset_for_save("save packet loaded");
        return result;
    });
    mod.hook<rac1::Level_LoadGameplay>([](auto original, int32_t full) -> uint32_t {
        const uint32_t result = original(full);
        reset_streak("level loaded");
        s_have_health = false;
        s_deaths_this_tick.clear();
        invalidate_hud();
        return result;
    });
    mod.hook<rac1::Hero_TakeDamage>([](auto original, int32_t amount) {
        const int32_t before = health();
        original(amount);
        if (amount > 0 && health() < before) reset_streak("Ratchet took damage");
    });
    mod.hook<rac1::Moby_RecordDeath>([](auto original, uint32_t moby,
                                        int32_t flags, int32_t index) {
        // Capture before the game's original code may delete or reuse it.
        const int16_t moby_class = s_mod->memory().read<int16_t>(moby + kMobyClassOffset);
        original(moby, flags, index);
        if (!active(current_cycle()) || !rules::is_hostile_class(moby_class)) return;
        if (!s_deaths_this_tick.insert(moby).second) return;
        const int old_multiplier = s_streak.multiplier();
        s_streak.kill();
        const int new_multiplier = s_streak.multiplier();
        if (new_multiplier != old_multiplier) {
            log_change("x" + std::to_string(new_multiplier) + " ("
                       + std::to_string(s_streak.kills()) + " consecutive kills)");
        }
    });
    mod.hook<rac1::Moby_CollectBolt>([](auto original, uint32_t bolt) {
        const int32_t before = s_mod->bolts(0);
        original(bolt);
        if (!active(current_cycle())) return;
        const int32_t after = s_mod->bolts(0);
        const int32_t extra = rules::extra_bolts(after - before,
                                                 s_streak.multiplier(), after);
        if (extra > 0) s_mod->bolts(extra);
    });
    mod.on_frame(frame);
    mod.info("Bolt Multiplier v0.2.2 (native C++), mode=" + s_mode);
}

} // namespace

OPENRAC_MOD_CPP("bolt-multiplier", "0.2.2", load)
