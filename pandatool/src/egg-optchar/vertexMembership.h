// Filename: vertexMembership.h
// Created by:  drose (21Jul03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef VERTEXMEMBERSHIP_H
#define VERTEXMEMBERSHIP_H

#include "pandatoolbase.h"

#include "pvector.h"

class EggGroup;

////////////////////////////////////////////////////////////////////
//       Class : VertexMembership
// Description : This class is used to help EggOptchar quantize the
//               membership of one vertex among its various groups.
////////////////////////////////////////////////////////////////////
class VertexMembership {
public:
  INLINE VertexMembership(EggGroup *group, double membership);
  INLINE VertexMembership(const VertexMembership &copy);
  INLINE void operator = (const VertexMembership &copy);

  INLINE bool operator < (const VertexMembership &other) const;

  EggGroup *_group;
  double _membership;
};

typedef pvector<VertexMembership> VertexMemberships;

#include "vertexMembership.I"

#endif

