// Filename: dxfVertexMap.h
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

#ifndef DXFVERTEXMAP_H
#define DXFVERTEXMAP_H

#include "pandatoolbase.h"

#include "dxfVertex.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
// 	 Class : DXFVertexMap
// Description : This is a map of DXFVertex to an integer index
//               number.  It is intended to be used to collapse
//               together identical vertices, since the DXF file has
//               no inherent notion of shared vertices.
////////////////////////////////////////////////////////////////////
class DXFVertexMap : public pmap<DXFVertex, int> {
public:
  int get_vertex_index(const DXFVertex &vert);
};

#endif
