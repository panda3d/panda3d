// Filename: xFileVertex.h
// Created by:  drose (19Jun01)
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

#ifndef XFILEVERTEX_H
#define XFILEVERTEX_H

#include "pandatoolbase.h"
#include "luse.h"

class EggVertex;
class EggPrimitive;

////////////////////////////////////////////////////////////////////
//       Class : XFileVertex
// Description : This represents a single vertex associated with an
//               XFileFace.
////////////////////////////////////////////////////////////////////
class XFileVertex {
public:
  XFileVertex(EggVertex *egg_vertex, EggPrimitive *egg_poly);
  int compare_to(const XFileVertex &other) const;

  Vertexf _point;
  TexCoordf _uv;
  Colorf _color;
};

#endif

