// Filename: pta_LVecBase3.h
// Created by:  drose (27Feb10)
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

#ifndef PTA_LVECBASE3_H
#define PTA_LVECBASE3_H

#include "pandabase.h"
#include "luse.h"
#include "pointerToArray.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_LVecBase3f
// Description : A pta of LVecBase3fs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
//
//               We actually wrap UnalignedLVecBase3f, in case we are
//               building with SSE2 and LVecBase3f requires strict
//               alignment.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLVecBase3f> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLVecBase3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLVecBase3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLVecBase3f>)

typedef PointerToArray<UnalignedLVecBase3f> PTA_LVecBase3f;
typedef ConstPointerToArray<UnalignedLVecBase3f> CPTA_LVecBase3f;

////////////////////////////////////////////////////////////////////
//       Class : PTA_LVecBase3d
// Description : A pta of LVecBase3ds.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
//
//               We actually wrap UnalignedLVecBase3d, in case we are
//               building with SSE2 and LVecBase3d requires strict
//               alignment.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLVecBase3d> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLVecBase3d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLVecBase3d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLVecBase3d>)

typedef PointerToArray<UnalignedLVecBase3d> PTA_LVecBase3d;
typedef ConstPointerToArray<UnalignedLVecBase3d> CPTA_LVecBase3d;

#ifndef STDFLOAT_DOUBLE
typedef PTA_LVecBase3f PTA_LVecBase3;
typedef CPTA_LVecBase3f CPTA_LVecBase3;
#else
typedef PTA_LVecBase3d PTA_LVecBase3;
typedef CPTA_LVecBase3d CPTA_LVecBase3;
#endif  // STDFLOAT_DOUBLE

// Bogus typedefs for interrogate and legacy Python code.
#ifdef CPPPARSER
typedef PTA_LVecBase3f PTAVecBase3f;
typedef CPTA_LVecBase3f CPTAVecBase3f;
typedef PTA_LVecBase3d PTAVecBase3d;
typedef CPTA_LVecBase3d CPTAVecBase3d;
#endif  // CPPPARSER

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
