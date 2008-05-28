// Filename: portalMask.h
// Created by:  masad (13May04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PORTALMASK_H
#define PORTALMASK_H

#include "pandabase.h"

#include "bitMask.h"

// This is the data type of the collision mask: the set of bits that
// every CollisionNode has, and that any two nodes must have some in
// common in order to be tested for a mutual intersection.

// This file is templated from collideMask.h, hence it is here


typedef BitMask32 PortalMask;

#endif

