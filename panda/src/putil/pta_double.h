// Filename: pta_double.h
// Created by:  drose (10May00)
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

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, RefCountObj<vector_double>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<RefCountObj<vector_double> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToArray<double>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerToArray<double>)

typedef PointerToArray<double> PTA_double;
typedef ConstPointerToArray<double> CPTA_double;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
