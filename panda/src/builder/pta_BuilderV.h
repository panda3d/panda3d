// Filename: pta_BuilderV.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_BUILDERV_H
#define PTA_BUILDERV_H

#include <pandabase.h>

#include "vector_BuilderV.h"

#include <pointerToArray.h>

////////////////////////////////////////////////////////////////////
//       Class : PTA_BuilderV
// Description : A pta of BuilderVs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, RefCountObj<vector_BuilderV>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToBase<RefCountObj<vector_BuilderV> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToArray<BuilderV>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, ConstPointerToArray<BuilderV>)

typedef PointerToArray<BuilderV> PTA_BuilderV;
typedef ConstPointerToArray<BuilderV> CPTA_BuilderV;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
