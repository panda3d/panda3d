// Filename: pta_int.h
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

#ifndef PTA_INT_H
#define PTA_INT_H

#include "pandabase.h"

#include "pointerToArray.h"
#include "vector_int.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_int
// Description : A pta of ints.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, RefCountObj<vector_int>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, PointerToBase<RefCountObj<vector_int> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, PointerToArray<int>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL, ConstPointerToArray<int>)

BEGIN_PUBLISH
typedef PointerToArray<int> PTA_int;
typedef ConstPointerToArray<int> CPTA_int;
END_PUBLISH

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
