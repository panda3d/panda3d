// Filename: drawMask.h
// Created by:  drose (13Mar02)
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

#ifndef DRAWMASK_H
#define DRAWMASK_H

#include "pandabase.h"

#include "bitMask.h"

// This is the data type of the draw mask: the set of bits that every
// PandaNode has, as well as a Camera, and that a node must have at
// least some bits in common with the current Camera in order to be
// visible.

typedef BitMask32 DrawMask;

#endif

