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

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<NodeRelation *>)
typedef vector<NodeRelation *> vector_NodeRelation_star;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
