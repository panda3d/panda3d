// Filename: collideMask.h
// Created by:  drose (03Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLIDEMASK_H
#define COLLIDEMASK_H

#include <pandabase.h>

#include <bitMask.h>

// This is the data type of the collision mask: the set of bits that
// every CollisionNode has, and that any two nodes must have some in
// common in order to be tested for a mutual intersection.

typedef BitMask32 CollideMask;

#endif

