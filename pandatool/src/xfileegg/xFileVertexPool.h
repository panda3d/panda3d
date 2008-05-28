// Filename: xFileVertexPool.h
// Created by:  drose (19Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef XFILEVERTEXPOOL_H
#define XFILEVERTEXPOOL_H

#include "pandatoolbase.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileVertexPool
// Description : This is a collection of unique vertices as extracted
//               out of a Geom or a series of Geoms.
////////////////////////////////////////////////////////////////////
class XFileVertexPool {
public:
  XFileVertexPool();
  ~XFileVertexPool();

  int add_vertex(const XFileVertex &vertex);

  int get_num_vertices();
  const Vertexf *get_vertices();
  const Normalf *get_normals();
  const TexCoordf *get_uvs();
  const Colorf *get_colors();

  

  void set_normal(const Normalf &normal);
  void set_uv(const TexCoordf &uv);
  void set_color(const Colorf &color);

  bool operator < (const XFileVertexPool &other) const;

private:
  Vertexf _point;
  Normalf _normal;
  TexCoordf _uv;
  Colorf _color;
};

#endif

