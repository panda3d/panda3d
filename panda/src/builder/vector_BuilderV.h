// Filename: vector_BuilderV.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_BUILDERV_H
#define VECTOR_BUILDERV_H

#include <pandabase.h>

#include "builderTypes.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_BuilderV
// Description : A vector of BuilderVs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, std::vector<BuilderV>)
typedef vector<BuilderV> vector_BuilderV;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
