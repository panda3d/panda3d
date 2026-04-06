/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LMatrix3_ext.h
 * @author rdb
 * @date 2015-02-25
 */

#ifndef PTA_LMATRIX3_EXT_H
#define PTA_LMATRIX3_EXT_H

#include "pointerToArray_ext.h"
#include "pta_LMatrix3.h"

#if defined(_MSC_VER) && !defined(CPPPARSER)
template class EXPORT_THIS Extension<PTA_LMatrix3f>;
template class EXPORT_THIS Extension<PTA_LMatrix3d>;
template class EXPORT_THIS Extension<CPTA_LMatrix3f>;
template class EXPORT_THIS Extension<CPTA_LMatrix3d>;
#endif

#endif
