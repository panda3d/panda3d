// Filename: pta_double.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_DOUBLE_H
#define PTA_DOUBLE_H

#include <pandabase.h>

#include "pointerToArray.h"
#include "vector_double.h"

////////////////////////////////////////////////////////////////////
//       Class : PTA_double
// Description : A pta of doubles.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, RefCountObj<vector_double>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<RefCountObj<vector_double> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToArray<double>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerToArray<double>)

typedef PointerToArray<double> PTA_double;
typedef ConstPointerToArray<double> CPTA_double;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
