// Filename: pta_uchar.h
// Created by:  drose (10May00)
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

#ifndef PTA_UCHAR_H
#define PTA_UCHAR_H

#include "pandabase.h"

#include "pointerToArray.h"
#include "vector_uchar.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_uchar
// Description : A pta of uchars.  This class is defined once here,
//               and exported to PANDAEXPRESS.DLL; other packages that
//               want to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

#if !defined(__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 7
// GCC 4.6 has a weird bug related to this type.
#else
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerToBase<ReferenceCountedVector<uchar> >)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerToArrayBase<uchar>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerToArray<unsigned char>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, ConstPointerToArray<unsigned char>)
#endif

typedef PointerToArray<unsigned char> PTA_uchar;
typedef ConstPointerToArray<unsigned char> CPTA_uchar;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
