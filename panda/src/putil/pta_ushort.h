// Filename: pta_ushort.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_USHORT_H
#define PTA_USHORT_H

#include <pandabase.h>

#include "pointerToArray.h"
#include "vector_ushort.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_ushort
// Description : A pta of ushorts.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, RefCountObj<vector_ushort>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<RefCountObj<vector_ushort> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToArray<unsigned short>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerToArray<unsigned short>)

typedef PointerToArray<unsigned short> PTA_ushort;
typedef ConstPointerToArray<unsigned short> CPTA_ushort;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
