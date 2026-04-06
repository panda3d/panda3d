/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bigEndian.h
 * @author drose
 * @date 2000-08-23
 */

#ifndef BIGENDIAN_H
#define BIGENDIAN_H

#include "dtoolbase.h"

#include "numeric_types.h"
#include "nativeNumericData.h"
#include "reversedNumericData.h"

/**
 * BigEndian is a special class that automatically reverses the byte-order of
 * numeric values for little-endian machines, and passes them through
 * unchanged for big-endian machines.
 */

#ifdef WORDS_BIGENDIAN
typedef NativeNumericData BigEndian;
#else
typedef ReversedNumericData BigEndian;
#endif

#endif
