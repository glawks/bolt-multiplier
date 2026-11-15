// SPDX-License-Identifier: GPL-3.0-or-later
#include "rules.hpp"

#include <algorithm>
#include <cstdint>

namespace bolt_multiplier {

bool is_hostile_class(int16_t moby_class) {
    return std::find(kHostileClasses.begin(), kHostileClasses.end(), moby_class)
        != kHostileClasses.end();
}

int multiplier_for_kills(int kills) {
    int value = 1;
    for (int index = 1; index < static_cast<int>(kKillThresholds.size()); ++index) {
        if (kills < kKillThresholds[index]) break;
        value = index + 1;
    }
    return value;
}

int32_t extra_bolts(int32_t gained, int multiplier, int32_t current_bolts) {
    if (gained <= 0 || multiplier <= 1 || current_bolts >= kMaximumBolts) return 0;
    const int64_t raw = int64_t{gained} * (multiplier - 1);
    const int64_t room = int64_t{kMaximumBolts} - current_bolts;
    return static_cast<int32_t>(std::max<int64_t>(0, std::min(raw, room)));
}

void Streak::reset() { m_kills = 0; }
void Streak::kill() { if (m_kills < kMaxKills) ++m_kills; }

} // namespace bolt_multiplier
