// Filename: vector_Vertexf.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_VERTEXF_H
#define VECTOR_VERTEXF_H

#include <pandabase.h>

#include "luse.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_Vertexf
// Description : A vector of Vertexfs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<Vertexf>)
typedef vector<Vertexf> vector_Vertexf;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
