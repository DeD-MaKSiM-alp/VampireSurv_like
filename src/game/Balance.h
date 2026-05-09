#pragma once

namespace balance
{

// XP threshold for the next level. Mirrors the GDD formula:
// round(40 * level^1.25). Level 1 -> 40, level 2 -> ~95, level 3 -> ~158, etc.
int xpToNextLevel(int level);

}
