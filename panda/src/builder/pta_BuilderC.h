// Filename: pta_BuilderC.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_BUILDERC_H
#define PTA_BUILDERC_H

#include <pandabase.h>

#include "vector_BuilderC.h"

#include <pointerToArray.h>

////////////////////////////////////////////////////////////////////
//       Class : PTA_BuilderC
// Description : A pta of BuilderCs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, RefCountObj<vector_BuilderC>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToBase<RefCountObj<vector_BuilderC> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToArray<BuilderC>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, ConstPointerToArray<BuilderC>)

typedef PointerToArray<BuilderC> PTA_BuilderC;
typedef ConstPointerToArray<BuilderC> CPTA_BuilderC;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
