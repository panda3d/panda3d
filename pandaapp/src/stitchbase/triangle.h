// Filename: triangle.h
// Created by:  drose (16Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <luse.h>

// A handy triangle utility.  Maybe more later.

// The triangle must be defined with vertices in counter-clockwise
// order.
bool
triangle_contains_point(const LPoint2d &p, const LPoint2d &v0,
                        const LPoint2d &v1, const LPoint2d &v2);

bool
triangle_contains_circle(const LPoint2d &p, double radius,
                         const LPoint2d &v0,
                         const LPoint2d &v1, const LPoint2d &v2);

#endif
