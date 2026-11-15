// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 glawks
#include "rules.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#define CHECK(expr) do { if (!(expr)) { std::cerr << "FAILED at line " << __LINE__ << ": " #expr "\n"; std::abort(); } } while (false)

int main() {
    using namespace bolt_multiplier;
    CHECK(kKillThresholds.front() == 0);
    CHECK(kKillThresholds.back() == 100);
    for (int i = 1; i < 20; ++i) {
        CHECK(kKillThresholds[i] > kKillThresholds[i - 1]);
        CHECK(multiplier_for_kills(kKillThresholds[i] - 1) == i);
        CHECK(multiplier_for_kills(kKillThresholds[i]) == i + 1);
    }
    CHECK(multiplier_for_kills(-10) == 1);
    CHECK(multiplier_for_kills(101) == 20);
    Streak streak;
    CHECK(streak.multiplier() == 1);
    streak.kill();
    CHECK(streak.multiplier() == 2);
    for (int i = 0; i < 400; ++i) streak.kill();
    CHECK(streak.multiplier() == 20);
    CHECK(streak.kills() == 100);
    streak.reset();
    CHECK(streak.kills() == 0 && streak.multiplier() == 1);

    // The same combo survives ordinary planet travel and repeated arrivals.
    for (int i = 0; i < 12; ++i) streak.kill();
    CHECK(streak.kills() == 12 && streak.multiplier() == 7);
    CHECK(!streak.on_event(StreakEvent::PlanetLoaded));
    CHECK(streak.kills() == 12 && streak.multiplier() == 7);
    CHECK(!streak.on_event(StreakEvent::PlanetLoaded));
    CHECK(streak.kills() == 12 && streak.multiplier() == 7);
    streak.kill();
    CHECK(streak.kills() == 13 && streak.multiplier() == 7);
    CHECK(extra_bolts(5, streak.multiplier(), 100) == 30);

    // Genuine run changes and any damage still reset to x1.
    for (StreakEvent event : {
             StreakEvent::NewGameOrSaveLoaded,
             StreakEvent::PlaythroughChanged,
             StreakEvent::Damage,
             StreakEvent::Death,
         }) {
        CHECK(streak.on_event(event));
        CHECK(streak.kills() == 0 && streak.multiplier() == 1);
        streak.kill();
        CHECK(streak.multiplier() == 2);
    }
    // A new planet immediately after damage must not revive the old streak.
    CHECK(!streak.on_event(StreakEvent::PlanetLoaded));
    CHECK(streak.kills() == 1 && streak.multiplier() == 2);
    CHECK(is_hostile_class(28));
    CHECK(is_hostile_class(1051));
    CHECK(is_hostile_class(1126));
    CHECK(is_hostile_class(1346));
    CHECK(!is_hostile_class(13)); // loose bolt
    CHECK(!is_hostile_class(437)); // harmless wildlife
    CHECK(!is_hostile_class(1401)); // scripted scenery
    CHECK(extra_bolts(5, 2, 30) == 5);
    CHECK(extra_bolts(20, 20, 30) == 380);
    CHECK(extra_bolts(10, 1, 30) == 0);
    CHECK(extra_bolts(0, 10, 30) == 0);
    CHECK(extra_bolts(100, 20, 9'999'990) == 9);
    CHECK(extra_bolts(100, 20, 9'999'999) == 0);
    CHECK(extra_bolts(2'000'000'000, 20, 1) == kMaximumBolts - 1);
    std::cout << "rules: all checks passed\n";
}
