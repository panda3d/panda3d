// Filename: collideMask.h
// Created by:  drose (03Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef COLLIDEMASK_H
#define COLLIDEMASK_H

#include "pandabase.h"

#include "bitMask.h"

// This is the data type of the collision mask: the set of bits that
// every CollisionNode has, and that any two nodes must have some in
// common in order to be tested for a mutual intersection.

// This file used to live in the collide directory, but since it's
// such a trivial definition that a few other directories (like egg)
// need without necessarily having to pull in all of collide, it
// seemed better to move it to putil.

typedef BitMask32 CollideMask;

// We need some conventions for initial bits for GeomNodes and
// CollideNodes.  These are primarily advisory, since the application
// programmer is free to define each bit as he or she chooses, but
// they also control the initial default values that are assigned to
// new nodes.

// By established convention, the lower 20 bits are reserved for
// CollisionNodes.  Each CollisionNode has all these bits set on by
// default (and no others).  You can (and probably should) change this
// on a per-node basis to specialize CollisionNodes for different
// purposes.
static const CollideMask default_collision_node_collide_mask = CollideMask::lower_on(20);

// The next bit is reserved for generic GeomNodes.  Each GeomNode has
// this bit on by default (and no others).  You can, of course, set
// any mask you want on a particular GeomNode; this is just the
// default bit if you choose not to do anything.
static const CollideMask default_geom_node_collide_mask = CollideMask::bit(20);

// The remaining 11 bits are presently unassigned.  No nodes will have
// these bits on by default.

#endif

