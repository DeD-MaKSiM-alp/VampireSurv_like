#include "game/Balance.h"

#include <algorithm>
#include <cmath>

namespace balance
{

int xpToNextLevel(int level)
{
    const int safeLevel = std::max(1, level);
    return static_cast<int>(std::round(40.0 * std::pow(static_cast<double>(safeLevel), 1.25)));
}

}
