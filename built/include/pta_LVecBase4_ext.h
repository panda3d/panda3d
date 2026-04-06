/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LVecBase4_ext.h
 * @author rdb
 * @date 2015-02-25
 */

#ifndef PTA_LVECBASE4_EXT_H
#define PTA_LVECBASE4_EXT_H

#include "pointerToArray_ext.h"
#include "pta_LVecBase4.h"

#if defined(_MSC_VER) && !defined(CPPPARSER)
template class EXPORT_THIS Extension<PTA_LVecBase4f>;
template class EXPORT_THIS Extension<PTA_LVecBase4d>;
template class EXPORT_THIS Extension<PTA_LVecBase4i>;
template class EXPORT_THIS Extension<CPTA_LVecBase4f>;
template class EXPORT_THIS Extension<CPTA_LVecBase4d>;
template class EXPORT_THIS Extension<CPTA_LVecBase4i>;
#endif

#endif
