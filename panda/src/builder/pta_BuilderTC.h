// Filename: pta_BuilderTC.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_BUILDERTC_H
#define PTA_BUILDERTC_H

#include <pandabase.h>

#include "vector_BuilderTC.h"

#include <pointerToArray.h>

////////////////////////////////////////////////////////////////////
//       Class : PTA_BuilderTC
// Description : A pta of BuilderTCs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, RefCountObj<vector_BuilderTC>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToBase<RefCountObj<vector_BuilderTC> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToArray<BuilderTC>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, ConstPointerToArray<BuilderTC>)

typedef PointerToArray<BuilderTC> PTA_BuilderTC;
typedef ConstPointerToArray<BuilderTC> CPTA_BuilderTC;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
