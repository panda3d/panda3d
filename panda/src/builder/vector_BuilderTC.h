// Filename: vector_BuilderTC.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_BUILDERTC_H
#define VECTOR_BUILDERTC_H

#include <pandabase.h>

#include "builderTypes.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_BuilderTC
// Description : A vector of BuilderTCs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, std::vector<BuilderTC>)
typedef vector<BuilderTC> vector_BuilderTC;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
