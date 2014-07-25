// Filename: pta_LVecBase2.h
// Created by:  drose (27Feb10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 2D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PTA_LVECBASE2_H
#define PTA_LVECBASE2_H

#include "pandabase.h"
#include "luse.h"
#include "pointerToArray.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_LVecBase2f
// Description : A pta of LVecBase2fs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LVecBase2f> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LVecBase2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LVecBase2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LVecBase2f>)

typedef PointerToArray<LVecBase2f> PTA_LVecBase2f;
typedef ConstPointerToArray<LVecBase2f> CPTA_LVecBase2f;

////////////////////////////////////////////////////////////////////
//       Class : PTA_LVecBase2d
// Description : A pta of LVecBase2ds.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LVecBase2d> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LVecBase2d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LVecBase2d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LVecBase2d>)

typedef PointerToArray<LVecBase2d> PTA_LVecBase2d;
typedef ConstPointerToArray<LVecBase2d> CPTA_LVecBase2d;

////////////////////////////////////////////////////////////////////
//       Class : PTA_LVecBase2i
// Description : A pta of LVecBase2is.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LVecBase2i> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LVecBase2i>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LVecBase2i>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LVecBase2i>)

typedef PointerToArray<LVecBase2i> PTA_LVecBase2i;
typedef ConstPointerToArray<LVecBase2i> CPTA_LVecBase2i;

#ifndef STDFLOAT_DOUBLE
typedef PTA_LVecBase2f PTA_LVecBase2;
typedef CPTA_LVecBase2f CPTA_LVecBase2;
#else
typedef PTA_LVecBase2d PTA_LVecBase2;
typedef CPTA_LVecBase2d CPTA_LVecBase2;
#endif  // STDFLOAT_DOUBLE

// Bogus typedefs for interrogate and legacy Python code.
#ifdef CPPPARSER
typedef PTA_LVecBase2f PTAVecBase2f;
typedef CPTA_LVecBase2f CPTAVecBase2f;
typedef PTA_LVecBase2d PTAVecBase2d;
typedef CPTA_LVecBase2d CPTAVecBase2d;
#endif  // CPPPARSER

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
