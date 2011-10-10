// Filename: pta_double.h
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

#ifndef PTA_DOUBLE_H
#define PTA_DOUBLE_H

#include "pandabase.h"

#include "pointerToArray.h"
#include "vector_double.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_double
// Description : A pta of doubles.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerToBase<ReferenceCountedVector<double> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerToArrayBase<double>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerToArray<double>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, ConstPointerToArray<double>)

typedef PointerToArray<double> PTA_double;
typedef ConstPointerToArray<double> CPTA_double;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
