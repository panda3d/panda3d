// Filename: collideMask.h
// Created by:  drose (03Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLIDEMASK_H
#define COLLIDEMASK_H

#include <pandabase.h>

#include "bitMask.h"

// This is the data type of the collision mask: the set of bits that
// every CollisionNode has, and that any two nodes must have some in
// common in order to be tested for a mutual intersection.

// This file used to live in the collide directory, but since it's
// such a trivial definition that a few other directories (like egg)
// need without necessarily having to pull in all of collide, it
// seemed better to move it to putil.

typedef BitMask32 CollideMask;

#endif

