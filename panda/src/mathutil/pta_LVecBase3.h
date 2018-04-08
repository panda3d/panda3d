/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LVecBase3.h
 * @author drose
 * @date 2010-02-27
 */

#ifndef PTA_LVECBASE3_H
#define PTA_LVECBASE3_H

#include "pandabase.h"
#include "luse.h"
#include "pointerToArray.h"

/**
 * A pta of LVecBase3fs.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LVecBase3f> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LVecBase3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LVecBase3f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LVecBase3f>)

typedef PointerToArray<LVecBase3f> PTA_LVecBase3f;
typedef ConstPointerToArray<LVecBase3f> CPTA_LVecBase3f;

/**
 * A pta of LVecBase3ds.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LVecBase3d> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LVecBase3d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LVecBase3d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LVecBase3d>)

typedef PointerToArray<LVecBase3d> PTA_LVecBase3d;
typedef ConstPointerToArray<LVecBase3d> CPTA_LVecBase3d;

/**
 * A pta of LVecBase3is.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<LVecBase3i> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<LVecBase3i>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<LVecBase3i>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<LVecBase3i>)

typedef PointerToArray<LVecBase3i> PTA_LVecBase3i;
typedef ConstPointerToArray<LVecBase3i> CPTA_LVecBase3i;

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

#endif
