// Filename: pta_LVecBase4f.h
// Created by:  drose (27Feb10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 4D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PTA_LVECBASE4F_H
#define PTA_LVECBASE4F_H

#include "pandabase.h"
#include "luse.h"
#include "pointerToArray.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_LVecBase4f
// Description : A pta of LVecBase4fs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LVecBase4f> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LVecBase4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LVecBase4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LVecBase4f>)

typedef PointerToArray<LVecBase4f> PTA_LVecBase4f;
typedef ConstPointerToArray<LVecBase4f> CPTA_LVecBase4f;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
