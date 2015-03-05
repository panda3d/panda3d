// Filename: pta_LVecBase3_ext.h
// Created by:  rdb (25Feb15)
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

#ifndef PTA_LVECBASE3_EXT_H
#define PTA_LVECBASE3_EXT_H

#include "pointerToArray_ext.h"
#include "pta_LVecBase3.h"

#if defined(_MSC_VER) && !defined(CPPPARSER)
template class EXPORT_THIS Extension<PTA_LVecBase3f>;
template class EXPORT_THIS Extension<PTA_LVecBase3d>;
template class EXPORT_THIS Extension<PTA_LVecBase3i>;
template class EXPORT_THIS Extension<CPTA_LVecBase3f>;
template class EXPORT_THIS Extension<CPTA_LVecBase3d>;
template class EXPORT_THIS Extension<CPTA_LVecBase3i>;
#endif

#endif
