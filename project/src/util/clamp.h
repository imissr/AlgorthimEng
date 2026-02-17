#ifndef ALGENG_UTIL_CLAMP_H
#define ALGENG_UTIL_CLAMP_H

#include <algorithm>
// keep pixel coordinates or values inside valid bounds
inline int clampInt(int v, int lo, int hi) {
    return std::max(lo, std::min(v, hi));
}

#endif
