// Filename: vector_NodeRelation_star.h
// Created by:  drose (07May01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
