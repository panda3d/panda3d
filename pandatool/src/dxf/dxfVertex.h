// Filename: dxfVertex.h
// Created by:  drose (04May04)
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

#ifndef DXFVERTEX_H
#define DXFVERTEX_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
// 	 Class : DXFVertex
// Description : Stored within DXFFile, this is the basic Vertex data
//               of a DXF file.  When DXFFile::DoneEntity() is called,
//               if the entity is a type to have vertices, then
//               DXFFile::_verts contains a list of all the vertices
//               that belong to the entity.
////////////////////////////////////////////////////////////////////
class DXFVertex {
public:
  DXFVertex() { }
  DXFVertex(const LPoint3d &p) : _p(p) { }
  int operator < (const DXFVertex &other) const;
  
  LPoint3d _p;
};

typedef pvector<DXFVertex> DXFVertices;

#endif

