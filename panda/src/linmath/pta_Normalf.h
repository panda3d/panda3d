// Filename: pta_Normalf.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_NORMALF_H
#define PTA_NORMALF_H

#include <pandabase.h>

#include "vector_Normalf.h"

#include <pointerToArray.h>

////////////////////////////////////////////////////////////////////
//       Class : PTA_Normalf
// Description : A pta of Normalfs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, RefCountObj<vector_Normalf>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<RefCountObj<vector_Normalf> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToArray<Normalf>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerToArray<Normalf>)

typedef PointerToArray<Normalf> PTA_Normalf;
typedef ConstPointerToArray<Normalf> CPTA_Normalf;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
