// Filename: vector_PT_Node.h
// Created by:  drose (16May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_PT_NODE_H
#define VECTOR_PT_NODE_H

#include <pandabase.h>

#include "node.h"
#include "pt_Node.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_PT_Node
// Description : A vector of PT(Node)'s.  This class is defined once
//               here, and exported to PANDA.DLL; other packages that
//               want to use a vector of this type (whether they need
//               to export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#ifdef HAVE_DINKUM
#define VV_PT_NODE std::_Vector_val<PT_Node, std::allocator<PT_Node> >
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VV_PT_NODE)
#endif
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<PT_Node>)
typedef vector<PT_Node> vector_PT_Node;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
