// Filename: littleEndian.h
// Created by:  drose (09Feb00)
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

#ifndef LITTLEENDIAN_H
#define LITTLEENDIAN_H

#include "pandabase.h"

#include "numeric_types.h"
#include "nativeNumericData.h"
#include "reversedNumericData.h"

////////////////////////////////////////////////////////////////////
//       Class : LittleEndian
// Description : LittleEndian is a special class that automatically
//               reverses the byte-order of numeric values for
//               big-endian machines, and passes them through
//               unchanged for little-endian machines.
////////////////////////////////////////////////////////////////////

#ifdef WORDS_BIGENDIAN
typedef ReversedNumericData LittleEndian;
#else
typedef NativeNumericData LittleEndian;
#endif

#endif
