/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2018 (git@dga.me.uk)
 */
#pragma once

#include "Vec2i.h"

namespace dw {

class DW_API Rect {
public:
    Vec2i begin, end;

    /// Constructors
    Rect();
    Rect(int x1, int y1, int x2, int y2);
    Rect(const Vec2i& _begin, const Vec2i& _end);

    /// Merge a rect into this one
    Rect& merge(const Rect& other);
};
}  // namespace dw
