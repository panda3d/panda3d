// Filename: eggToC.h
// Created by:  drose (03Aug01)
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

#ifndef EGGTOC_H
#define EGGTOC_H

#include "pandatoolbase.h"

#include "eggToSomething.h"

#include "pmap.h"

class EggNode;
class EggVertexPool;
class EggBin;

////////////////////////////////////////////////////////////////////
//       Class : EggToC
// Description :
////////////////////////////////////////////////////////////////////
class EggToC : public EggToSomething {
public:
  EggToC();

  void run();

  void traverse(EggNode *node);
  void write_vertex_pool(EggVertexPool *vpool);
  void write_bin(EggBin *bin);

  bool _vertices;
  bool _uvs;
  bool _vertex_normals;
  bool _vertex_colors;
  bool _polygons;
  bool _polygon_normals;
  bool _polygon_colors;

  bool _triangulate_polygons;

  typedef pmap<EggVertexPool *, int> VertexPools;
  VertexPools _vertex_pools;
  int _next_vpool_index;
  int _next_bin_index;
};

#endif
