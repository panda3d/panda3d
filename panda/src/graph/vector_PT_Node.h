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

#define EXPCL EXPCL_PANDA
#define EXPTP EXPTP_PANDA
#define TYPE PT_Node
#define NAME vector_PT_Node

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
