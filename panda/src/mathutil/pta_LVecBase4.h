/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LVecBase4.h
 * @author drose
 * @date 2010-02-27
 */

#ifndef PTA_LVECBASE4_H
#define PTA_LVECBASE4_H

#include "pandabase.h"
#include "luse.h"
#include "pointerToArray.h"

/**
 * A pta of LVecBase4fs.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 *
 * We actually wrap UnalignedLVecBase4f, in case we are building with SSE2 and
 * LVecBase4f requires strict alignment.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLVecBase4f> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLVecBase4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLVecBase4f>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLVecBase4f>)

typedef PointerToArray<UnalignedLVecBase4f> PTA_LVecBase4f;
typedef ConstPointerToArray<UnalignedLVecBase4f> CPTA_LVecBase4f;

/**
 * A pta of LVecBase4ds.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 *
 * We actually wrap UnalignedLVecBase4d, in case we are building with SSE2 and
 * LVecBase4d requires strict alignment.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLVecBase4d> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLVecBase4d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLVecBase4d>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLVecBase4d>)

typedef PointerToArray<UnalignedLVecBase4d> PTA_LVecBase4d;
typedef ConstPointerToArray<UnalignedLVecBase4d> CPTA_LVecBase4d;

/**
 * A pta of LVecBase4is.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a pta of this type (whether they
 * need to export it or not) should include this header file, rather than
 * defining the pta again.
 *
 * We actually wrap UnalignedLVecBase4i, in case we are building with SSE2 and
 * LVecBase4i requires strict alignment.
 */

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToBase<ReferenceCountedVector<UnalignedLVecBase4i> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArrayBase<UnalignedLVecBase4i>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, PointerToArray<UnalignedLVecBase4i>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_MATHUTIL, EXPTP_PANDA_MATHUTIL, ConstPointerToArray<UnalignedLVecBase4i>)

typedef PointerToArray<UnalignedLVecBase4i> PTA_LVecBase4i;
typedef ConstPointerToArray<UnalignedLVecBase4i> CPTA_LVecBase4i;

#ifndef STDFLOAT_DOUBLE
typedef PTA_LVecBase4f PTA_LVecBase4;
typedef CPTA_LVecBase4f CPTA_LVecBase4;
#else
typedef PTA_LVecBase4d PTA_LVecBase4;
typedef CPTA_LVecBase4d CPTA_LVecBase4;
#endif  // STDFLOAT_DOUBLE

// Bogus typedefs for interrogate and legacy Python code.
#ifdef CPPPARSER
typedef PTA_LVecBase4f PTAVecBase4f;
typedef CPTA_LVecBase4f CPTAVecBase4f;
typedef PTA_LVecBase4d PTAVecBase4d;
typedef CPTA_LVecBase4d CPTAVecBase4d;
#endif  // CPPPARSER

#endif
