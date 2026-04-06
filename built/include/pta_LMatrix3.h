/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LMatrix3.h
 * @author drose
 * @date 2010-02-27
 */

#ifndef PTA_LMATRIX3_H
#define PTA_LMATRIX3_H

#include "pandabase.h"
#include "luse.h"
#include "pointerToArray.h"

/**
 * A pta of LMatrix3fs.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LMatrix3f> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LMatrix3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LMatrix3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LMatrix3f>)

typedef PointerToArray<LMatrix3f> PTA_LMatrix3f;
typedef ConstPointerToArray<LMatrix3f> CPTA_LMatrix3f;

/**
 * A pta of LMatrix3ds.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LMatrix3d> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LMatrix3d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LMatrix3d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LMatrix3d>)

typedef PointerToArray<LMatrix3d> PTA_LMatrix3d;
typedef ConstPointerToArray<LMatrix3d> CPTA_LMatrix3d;

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

#endif
