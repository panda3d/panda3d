// Filename: portalMask.h
// Created by:  masad (13May04)
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

