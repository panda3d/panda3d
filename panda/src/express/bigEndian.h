// Filename: bigEndian.h
// Created by:  drose (23Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BIGENDIAN_H
#define BIGENDIAN_H

#include <pandabase.h>

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

#ifdef IS_LITTLE_ENDIAN
typedef ReversedNumericData BigEndian;
#else
typedef NativeNumericData BigEndian;
#endif

#endif
