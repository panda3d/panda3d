/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfVertex.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef DXFVERTEX_H
#define DXFVERTEX_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "luse.h"

/**
 * Stored within DXFFile, this is the basic Vertex data of a DXF file.  When
 * DXFFile::DoneEntity() is called, if the entity is a type to have vertices,
 * then DXFFile::_verts contains a list of all the vertices that belong to the
 * entity.
 */
class DXFVertex {
public:
  DXFVertex() { }
  DXFVertex(const LPoint3d &p) : _p(p) { }
  int operator < (const DXFVertex &other) const;

  LPoint3d _p;
};

typedef pvector<DXFVertex> DXFVertices;

#endif
