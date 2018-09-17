/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_uchar.h
 * @author drose
 * @date 2000-05-10
 */

#ifndef PTA_UCHAR_H
#define PTA_UCHAR_H

#include "pandabase.h"

#include "pointerToArray.h"
#include "vector_uchar.h"

/**
 * A pta of uchars.  This class is defined once here, and exported to
 * PANDA_EXPRESS.DLL; other packages that want to use a pta of this type
 * (whether they need to export it or not) should include this header file,
 * rather than defining the pta again.
 */

#if !defined(__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 7
// GCC 4.6 has a weird bug related to this type.
#else
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EXPRESS, EXPTP_PANDA_EXPRESS, PointerToBase<ReferenceCountedVector<uchar> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EXPRESS, EXPTP_PANDA_EXPRESS, PointerToArrayBase<uchar>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EXPRESS, EXPTP_PANDA_EXPRESS, PointerToArray<unsigned char>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_EXPRESS, EXPTP_PANDA_EXPRESS, ConstPointerToArray<unsigned char>)
#endif

typedef PointerToArray<unsigned char> PTA_uchar;
typedef ConstPointerToArray<unsigned char> CPTA_uchar;

#endif
