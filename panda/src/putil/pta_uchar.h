// Filename: pta_uchar.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_UCHAR_H
#define PTA_UCHAR_H

#include <pandabase.h>

#include "pointerToArray.h"
#include "vector_uchar.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_uchar
// Description : A pta of uchars.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, RefCountObj<vector_uchar>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<RefCountObj<vector_uchar> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToArray<unsigned char>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerToArray<unsigned char>)

typedef PointerToArray<unsigned char> PTA_uchar;
typedef ConstPointerToArray<unsigned char> CPTA_uchar;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
