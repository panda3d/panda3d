// Filename: vector_NodeRelation_star.h
// Created by:  drose (07May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_NODERELATION_STAR_H
#define VECTOR_NODERELATION_STAR_H

#include <pandabase.h>

#include <vector>

class NodeRelation;

////////////////////////////////////////////////////////////////////
//       Class : vector_NodeRelation_star
// Description : A vector of NodeRelation*'s.  This class is defined
//               once here, and exported to PANDA.DLL; other packages
//               that want to use a vector of this type (whether they
//               need to export it or not) should include this header
//               file, rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA
#define EXPTP EXPTP_PANDA
#define TYPE NodeRelation *
#define NAME vector_NodeRelation_star

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
