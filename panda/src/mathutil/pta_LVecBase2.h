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
//
//               We actually wrap UnalignedLVecBase2f, in case we are
//               building with SSE2 and LVecBase2f requires strict
//               alignment.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLVecBase2f> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLVecBase2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLVecBase2f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLVecBase2f>)

typedef PointerToArray<UnalignedLVecBase2f> PTA_LVecBase2f;
typedef ConstPointerToArray<UnalignedLVecBase2f> CPTA_LVecBase2f;

////////////////////////////////////////////////////////////////////
//       Class : PTA_LVecBase2d
// Description : A pta of LVecBase2ds.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
//
//               We actually wrap UnalignedLVecBase2d, in case we are
//               building with SSE2 and LVecBase2d requires strict
//               alignment.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLVecBase2d> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLVecBase2d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLVecBase2d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLVecBase2d>)

typedef PointerToArray<UnalignedLVecBase2d> PTA_LVecBase2d;
typedef ConstPointerToArray<UnalignedLVecBase2d> CPTA_LVecBase2d;

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
