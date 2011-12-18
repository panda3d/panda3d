// Filename: pta_LMatrix3.h
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

#ifndef PTA_LMATRIX3_H
#define PTA_LMATRIX3_H

#include "pandabase.h"
#include "luse.h"
#include "pointerToArray.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_LMatrix3f
// Description : A pta of LMatrix3fs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
//
//               We actually wrap UnalignedLMatrix3f, in case we are
//               building with SSE2 and LMatrix3f requires strict
//               alignment.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLMatrix3f> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLMatrix3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLMatrix3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLMatrix3f>)

typedef PointerToArray<UnalignedLMatrix3f> PTA_LMatrix3f;
typedef ConstPointerToArray<UnalignedLMatrix3f> CPTA_LMatrix3f;

////////////////////////////////////////////////////////////////////
//       Class : PTA_LMatrix3d
// Description : A pta of LMatrix3ds.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
//
//               We actually wrap UnalignedLMatrix3d, in case we are
//               building with SSE2 and LMatrix3d requires strict
//               alignment.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLMatrix3d> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLMatrix3d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLMatrix3d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLMatrix3d>)

typedef PointerToArray<UnalignedLMatrix3d> PTA_LMatrix3d;
typedef ConstPointerToArray<UnalignedLMatrix3d> CPTA_LMatrix3d;

#ifndef STDFLOAT_DOUBLE
typedef PTA_LMatrix3f PTA_LMatrix3;
typedef CPTA_LMatrix3f CPTA_LMatrix3;
#else
typedef PTA_LMatrix3d PTA_LMatrix3;
typedef CPTA_LMatrix3d CPTA_LMatrix3;
#endif  // STDFLOAT_DOUBLE

// Bogus typedefs for interrogate and legacy Python code.
#ifdef CPPPARSER
typedef PTA_LMatrix3 PTAMat3;
typedef CPTA_LMatrix3 CPTAMat3;
typedef PTA_LMatrix3d PTAMat3d;
typedef CPTA_LMatrix3d CPTAMat3d;
#endif  // CPPPARSER

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
