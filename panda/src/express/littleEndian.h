// Filename: littleEndian.h
// Created by:  drose (09Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef LITTLEENDIAN_H
#define LITTLEENDIAN_H

#include <pandabase.h>

#include "numeric_types.h"
#include "nativeNumericData.h"
#include "reversedNumericData.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LittleEndian
// Description : LittleEndian is a special class that automatically
//               reverses the byte-order of numeric values for
//               big-endian machines, and passes them through
//               unchanged for little-endian machines.
////////////////////////////////////////////////////////////////////

#ifdef IS_LITTLE_ENDIAN
typedef NativeNumericData LittleEndian;
#else
typedef ReversedNumericData LittleEndian;
#endif

#endif
