// Filename: eggMesherEdge.h
// Created by:  drose (13Mar05)
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

#ifndef EGGMESHEREDGE_H
#define EGGMESHEREDGE_H

#include "pandabase.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "plist.h"

class EggMesherStrip;

////////////////////////////////////////////////////////////////////
//       Class : EggMesherEdge
// Description : Represents one edge of a triangle, as used by the
//               EggMesher to discover connected triangles.  The edge
//               is actually represented as a pair of vertex indices
//               into the same vertex pool.
////////////////////////////////////////////////////////////////////
class EggMesherEdge {
public:
  INLINE EggMesherEdge(int vi_a, int vi_b);
  INLINE EggMesherEdge(const EggMesherEdge &copy);

  void remove(EggMesherStrip *strip);
  void change_strip(EggMesherStrip *from, EggMesherStrip *to);

  INLINE bool contains_vertex(int vi) const;

  INLINE bool matches(const EggMesherEdge &other) const;

  INLINE EggMesherEdge *common_ptr();

  INLINE bool operator == (const EggMesherEdge &other) const;
  INLINE bool operator != (const EggMesherEdge &other) const;
  INLINE bool operator < (const EggMesherEdge &other) const;

  INLINE double compute_length(const EggVertexPool *vertex_pool) const;
  INLINE LVecBase3d compute_box(const EggVertexPool *vertex_pool) const;

  void output(ostream &out) const;

  int _vi_a, _vi_b;

  typedef plist<EggMesherStrip *> Strips;
  Strips _strips;
  EggMesherEdge *_opposite;
};

INLINE ostream &
operator << (ostream &out, const EggMesherEdge &edge) {
  edge.output(out);
  return out;
}

#include "eggMesherEdge.I"

#endif
