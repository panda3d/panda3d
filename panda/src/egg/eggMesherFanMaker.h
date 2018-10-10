/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesherFanMaker.h
 * @author drose
 * @date 2005-03-22
 */

#ifndef EGGMESHERFANMAKER_H
#define EGGMESHERFANMAKER_H

#include "pandabase.h"

#include "eggMesherEdge.h"
#include "eggMesherStrip.h"
#include "config_egg.h"
#include "plist.h"
#include "pvector.h"

#include "pnotify.h"
#include "mathNumbers.h"

class EggMesher;

/**
 * This class is used by EggMesher::find_fans() to attempt to make an
 * EggTriangleFan out of the polygons connected to the indicated vertex.
 */
class EXPCL_PANDA_EGG EggMesherFanMaker {
public:
  typedef plist<const EggMesherEdge *> Edges;
  typedef plist<EggMesherStrip *> Strips;

  EggMesherFanMaker(int vertex, EggMesherStrip *tri,
                    EggMesher *mesher);
  EggMesherFanMaker(const EggMesherFanMaker &copy);
  void operator = (const EggMesherFanMaker &copy);

  INLINE bool operator < (const EggMesherFanMaker &other) const;
  INLINE bool operator != (const EggMesherFanMaker &other) const;
  INLINE bool operator == (const EggMesherFanMaker &other) const;

  INLINE bool is_empty() const;
  INLINE bool is_valid() const;
  INLINE bool is_coplanar_with(const EggMesherFanMaker &other) const;

  bool join(EggMesherFanMaker &other);
  double compute_angle() const;

  int build(EggGroupNode *unrolled_tris);
  int unroll(Strips::iterator strip_begin, Strips::iterator strip_end,
             Edges::iterator edge_begin, Edges::iterator edge_end,
             EggGroupNode *unrolled_tris);

  void output(std::ostream &out) const;

  int _vertex;
  Edges _edges;
  Strips _strips;
  bool _planar;
  EggMesher *_mesher;
};

INLINE std::ostream &operator << (std::ostream &out, const EggMesherFanMaker &fm);

#include "eggMesherFanMaker.I"

#endif
