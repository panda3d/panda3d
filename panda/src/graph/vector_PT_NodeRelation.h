// Filename: vector_PT_NodeRelation.h
// Created by:  drose (07May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_PT_NODERELATION_H
#define VECTOR_PT_NODERELATION_H

#include <pandabase.h>

#include "nodeRelation.h"
#include "pt_NodeRelation.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_PT_NodeRelation
// Description : A vector of PT(NodeRelation)'s.  This class is defined once
//               here, and exported to PANDA.DLL; other packages that
//               want to use a vector of this type (whether they need
//               to export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA
#define EXPTP EXPTP_PANDA
#define TYPE PT_NodeRelation
#define NAME vector_PT_NodeRelation

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
