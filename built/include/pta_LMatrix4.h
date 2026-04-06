/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LMatrix4.h
 * @author drose
 * @date 2010-02-27
 */

#ifndef PTA_LMATRIX4_H
#define PTA_LMATRIX4_H

#include "pandabase.h"
#include "luse.h"
#include "pointerToArray.h"

/**
 * A pta of LMatrix4fs.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 *
 * We actually wrap UnalignedLMatrix4f, in case we are building with SSE2 and
 * LMatrix4f requires strict alignment.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLMatrix4f> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLMatrix4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLMatrix4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLMatrix4f>)

typedef PointerToArray<UnalignedLMatrix4f> PTA_LMatrix4f;
typedef ConstPointerToArray<UnalignedLMatrix4f> CPTA_LMatrix4f;

/**
 * A pta of LMatrix4ds.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 *
 * We actually wrap UnalignedLMatrix4d, in case we are building with SSE2 and
 * LMatrix4d requires strict alignment.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLMatrix4d> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLMatrix4d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLMatrix4d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLMatrix4d>)

typedef PointerToArray<UnalignedLMatrix4d> PTA_LMatrix4d;
typedef ConstPointerToArray<UnalignedLMatrix4d> CPTA_LMatrix4d;

#ifndef STDFLOAT_DOUBLE
typedef PTA_LMatrix4f PTA_LMatrix4;
typedef CPTA_LMatrix4f CPTA_LMatrix4;
#else
typedef PTA_LMatrix4d PTA_LMatrix4;
typedef CPTA_LMatrix4d CPTA_LMatrix4;
#endif  // STDFLOAT_DOUBLE

// Bogus typedefs for interrogate and legacy Python code.
#ifdef CPPPARSER
typedef PTA_LMatrix4 PTAMat4;
typedef CPTA_LMatrix4 CPTAMat4;
typedef PTA_LMatrix4d PTAMat4d;
typedef CPTA_LMatrix4d CPTAMat4d;
#endif  // CPPPARSER

#endif
