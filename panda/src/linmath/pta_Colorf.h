// Filename: pta_Colorf.h
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

#ifndef PTA_COLORF_H
#define PTA_COLORF_H

#include "pandabase.h"

#include "vector_Colorf.h"

#include "pointerToArray.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_Colorf
// Description : A pta of Colorfs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, RefCountObj<vector_Colorf>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<RefCountObj<vector_Colorf> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToArray<Colorf>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerToArray<Colorf>)

typedef PointerToArray<Colorf> PTA_Colorf;
typedef ConstPointerToArray<Colorf> CPTA_Colorf;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
