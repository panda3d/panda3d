// Filename: pta_BuilderN.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_BUILDERN_H
#define PTA_BUILDERN_H

#include <pandabase.h>

#include "vector_BuilderN.h"

#include <pointerToArray.h>

////////////////////////////////////////////////////////////////////
//       Class : PTA_BuilderN
// Description : A pta of BuilderNs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, RefCountObj<vector_BuilderN>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToBase<RefCountObj<vector_BuilderN> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToArray<BuilderN>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, ConstPointerToArray<BuilderN>)

typedef PointerToArray<BuilderN> PTA_BuilderN;
typedef ConstPointerToArray<BuilderN> CPTA_BuilderN;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
