// Filename: pta_ushort.h
// Created by:  drose (10May00)
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

#ifndef PTA_USHORT_H
#define PTA_USHORT_H

#include "pandabase.h"

#include "pointerToArray.h"
#include "vector_ushort.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_ushort
// Description : A pta of ushorts.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, PointerToBase<ReferenceCountedVector<ushort> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, PointerToArrayBase<ushort>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, PointerToArray<unsigned short>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ConstPointerToArray<unsigned short>)

typedef PointerToArray<unsigned short> PTA_ushort;
typedef ConstPointerToArray<unsigned short> CPTA_ushort;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
