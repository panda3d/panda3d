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

#define EXPCL EXPCL_PANDA 
#define EXPTP EXPTP_PANDA 
#define TYPE Vertexf
#define NAME vector_Vertexf

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
