// Filename: bigEndian.h
// Created by:  drose (23Aug00)
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

#ifndef BIGENDIAN_H
#define BIGENDIAN_H

#include "pandabase.h"

#include "numeric_types.h"
#include "nativeNumericData.h"
#include "reversedNumericData.h"

////////////////////////////////////////////////////////////////////
//       Class : BigEndian
// Description : BigEndian is a special class that automatically
//               reverses the byte-order of numeric values for
//               little-endian machines, and passes them through
//               unchanged for big-endian machines.
////////////////////////////////////////////////////////////////////

#ifdef WORDS_BIGENDIAN
typedef NativeNumericData BigEndian;
#else
typedef ReversedNumericData BigEndian;
#endif

#endif
