// Filename: vector_BuilderN.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_BUILDERN_H
#define VECTOR_BUILDERN_H

#include <pandabase.h>

#include "builderTypes.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_BuilderN
// Description : A vector of BuilderNs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, std::vector<BuilderN>)
typedef vector<BuilderN> vector_BuilderN;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
