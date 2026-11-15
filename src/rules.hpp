// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 glawks
// Portions of the enemy classification are adapted from OpenRAC's Difficulty mod.
// Copyright (c) 2026 the OpenRAC contributors
// Pure, host-independent multiplier rules for Ratchet & Clank 1.
#pragma once

#include <array>
#include <cstdint>

namespace bolt_multiplier {

constexpr int kMaxMultiplier = 20;
constexpr int kMaxKills = 100;
constexpr int32_t kMaximumBolts = 9'999'999;

// The progression used in the original design, from x1 through x20.
inline constexpr std::array<int, 20> kKillThresholds = {
    0, 1, 2, 4, 6, 9, 12, 16, 20, 25,
    30, 36, 42, 49, 56, 64, 72, 81, 90, 100
};

// These classes are the hostile enemy/turret/roamer/boss roles in OpenRAC's
// Difficulty mod's class catalogue, excluding crates, NPCs and scenery.
// Keep this list reviewed when the upstream catalogue changes.
inline constexpr std::array<int16_t, 56> kHostileClasses = {
    28, 29, 30, 31, 35, 36, 44, 52, 63, 123, 133, 211, 217, 252,
    253, 294, 326, 333, 340, 344, 347, 427, 459, 563, 572, 573,
    577, 578, 580, 623, 625, 631, 638, 749, 865, 866, 1023,
    1048, 1051, 1059, 1068, 1106, 1126, 1130, 1196, 1202, 1231,
    1246, 1262, 1346, 1371, 1382, 1422, 1445, 1454, 1564
};

bool is_hostile_class(int16_t moby_class);
int multiplier_for_kills(int kills);
int32_t extra_bolts(int32_t gained, int multiplier, int32_t current_bolts);

// A new planet is still the same run. Other events start a fresh streak.
// Keep the rule here so the event lifecycle can be tested without the game.
enum class StreakEvent {
    PlanetLoaded,
    NewGameOrSaveLoaded,
    PlaythroughChanged,
    Damage,
    Death,
};

class Streak {
public:
    // Returns true when this event resets the streak.
    bool on_event(StreakEvent event);
    void reset();
    void kill();
    int kills() const { return m_kills; }
    int multiplier() const { return multiplier_for_kills(m_kills); }
private:
    int m_kills = 0;
};

} // namespace bolt_multiplier
