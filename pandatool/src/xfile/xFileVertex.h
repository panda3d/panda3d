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

////////////////////////////////////////////////////////////////////
//       Class : XFileVertex
// Description : This class represents a single Vertex as extracted
//               from a Geom and added to the vertex pool.
////////////////////////////////////////////////////////////////////
class XFileVertex {
public:
  XFileVertex(const Vertexf &point);

  void set_normal(const Normalf &normal);
  void set_uv(const TexCoordf &uv);
  void set_color(const Colorf &color);

  bool operator < (const XFileVertex &other) const;

private:
  Vertexf _point;
  Normalf _normal;
  TexCoordf _uv;
  Colorf _color;
};

#endif

