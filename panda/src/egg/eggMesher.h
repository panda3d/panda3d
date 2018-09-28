/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesher.h
 * @author drose
 * @date 2005-03-13
 */

#ifndef EGGMESHER_H
#define EGGMESHER_H

#include "pandabase.h"
#include "eggMesherEdge.h"
#include "eggMesherStrip.h"
#include "eggPolygon.h"
#include "pvector.h"
#include "plist.h"
#include "pset.h"
#include "pmap.h"

#include <algorithm>

/**
 * Collects together unrelated EggPrimitives, determines their edge
 * connectivity, and generates a set of EggTriangleStrips that represent the
 * same geometry.
 */
class EXPCL_PANDA_EGG EggMesher {
public:
  EggMesher();

  void mesh(EggGroupNode *group, bool flat_shaded);

  void write(std::ostream &out) const;

  bool _consider_fans;
  bool _retesselate_coplanar;
  bool _show_quads;
  bool _show_qsheets;

private:
  void clear();
  bool add_polygon(const EggPolygon *egg_poly,
                   EggMesherStrip::MesherOrigin origin);
  void do_mesh();
  PT(EggPrimitive) get_prim(EggMesherStrip &strip);

  typedef plist<EggMesherStrip> Strips;
  typedef pset<EggMesherEdge> Edges;
  typedef pset<EggMesherEdge *> EdgePtrs;
  typedef pmap<int, EdgePtrs> Verts;

  // This is used for show-qsheets.
  typedef pmap<int, LColor> ColorSheetMap;

  int count_vert_edges(const EdgePtrs &edges) const;
  plist<EggMesherStrip> &choose_strip_list(const EggMesherStrip &strip);

  void build_sheets();
  void find_fans();
  void make_quads();
  void mesh_list(Strips &strips);
  static void make_random_color(LColor &color);

  bool _flat_shaded;
  Strips _tris, _quads, _strips;
  Strips _dead, _done;
  Verts _verts;
  Edges _edges;
  int _strip_index;
  EggVertexPool *_vertex_pool;
  ColorSheetMap _color_sheets;

  friend class EggMesherStrip;
  friend class EggMesherFanMaker;
};

#include "eggMesher.I"

#endif
