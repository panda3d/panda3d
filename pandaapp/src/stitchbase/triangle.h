// Filename: triangle.h
// Created by:  drose (16Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "luse.h"

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
