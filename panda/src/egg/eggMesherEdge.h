/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesherEdge.h
 * @author drose
 * @date 2005-03-13
 */

#ifndef EGGMESHEREDGE_H
#define EGGMESHEREDGE_H

#include "pandabase.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "plist.h"

class EggMesherStrip;

/**
 * Represents one edge of a triangle, as used by the EggMesher to discover
 * connected triangles.  The edge is actually represented as a pair of vertex
 * indices into the same vertex pool.
 */
class EXPCL_PANDA_EGG EggMesherEdge {
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

  void output(std::ostream &out) const;

  int _vi_a, _vi_b;

  typedef plist<EggMesherStrip *> Strips;
  Strips _strips;
  EggMesherEdge *_opposite;
};

INLINE std::ostream &
operator << (std::ostream &out, const EggMesherEdge &edge) {
  edge.output(out);
  return out;
}

#include "eggMesherEdge.I"

#endif
