// Filename: bitMask32Transition.h
// Created by:  drose (08Jun00)
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

#ifndef BITMASK32TRANSITION_H
#define BITMASK32TRANSITION_H

#include <pandabase.h>

#include "bitMaskTransition.h"

#include <bitMask.h>


////////////////////////////////////////////////////////////////////
//       Class : BitMask32Transition
// Description : This is just an instantation of BitMaskTransition
//               using BitMask32, the most common bitmask type.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, BitMaskTransition<BitMask32>);

typedef BitMaskTransition<BitMask32> BitMask32Transition;

#ifdef __GNUC__
#pragma interface
#endif

#endif


