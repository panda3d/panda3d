/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileVertexPool.h
 * @author drose
 * @date 2001-06-19
 */

#ifndef XFILEVERTEXPOOL_H
#define XFILEVERTEXPOOL_H

#include "pandatoolbase.h"

/**
 * This is a collection of unique vertices as extracted out of a Geom or a
 * series of Geoms.
 */
class XFileVertexPool {
public:
  XFileVertexPool();
  ~XFileVertexPool();

  int add_vertex(const XFileVertex &vertex);

  int get_num_vertices();
  const LVertex *get_vertices();
  const LNormal *get_normals();
  const LTexCoord *get_uvs();
  const LColor *get_colors();



  void set_normal(const LNormal &normal);
  void set_uv(const LTexCoord &uv);
  void set_color(const LColor &color);

  bool operator < (const XFileVertexPool &other) const;

private:
  LVertex _point;
  LNormal _normal;
  LTexCoord _uv;
  LColor _color;
};

#endif
