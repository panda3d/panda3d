// Filename: pta_uchar.h
// Created by:  drose (10May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, RefCountObj<vector_uchar>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerToBase<RefCountObj<vector_uchar> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, PointerToArray<unsigned char>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS, ConstPointerToArray<unsigned char>)

typedef PointerToArray<unsigned char> PTA_uchar;
typedef ConstPointerToArray<unsigned char> CPTA_uchar;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
