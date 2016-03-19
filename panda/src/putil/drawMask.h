/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file drawMask.h
 * @author drose
 * @date 2002-03-13
 */

#ifndef DRAWMASK_H
#define DRAWMASK_H

#include "pandabase.h"

#include "bitMask.h"

// This is the data type of the draw mask: the set of bits that every
// PandaNode has, as well as a Camera, and that a node must have at least some
// bits in common with the current Camera in order to be visible.

typedef BitMask32 DrawMask;

#endif
