// Filename: eggMesher.h
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

#ifndef EGGMESHER_H
#define EGGMESHER_H

#include "pandabase.h"
#include "eggMesherEdge.h"
#include "eggMesherStrip.h"
//#include "eggMesherFanMaker.h"
#include "eggPolygon.h"
#include "pvector.h"
#include "plist.h"
#include "pset.h"
#include "pmap.h"

#include <algorithm>

class MesherFanMaker;

///////////////////////////////////////////////////////////////////
//       Class : EggMesher
// Description : Collects together unrelated EggPrimitives, determines
//               their edge connectivity, and generates a set of
//               EggTriangleStrips that represent the same geometry.
////////////////////////////////////////////////////////////////////
class EggMesher {
public:
  EggMesher();

  void mesh(EggGroupNode *group);

  void write(ostream &out) const;

  bool _consider_fans;
  bool _retesselate_coplanar;
  bool _show_quads;
  bool _show_qsheets;

private:
  bool add_polygon(const EggPolygon *egg_poly, 
                   EggMesherStrip::MesherOrigin origin);
  void do_mesh();
  PT(EggPrimitive) get_prim(EggMesherStrip &strip);

  typedef plist<EggMesherStrip> Strips;
  typedef pset<EggMesherEdge> Edges;
  typedef pset<EggMesherEdge *> EdgePtrs;
  typedef pmap<int, EdgePtrs> Verts;

  // This is used for show-qsheets.
  typedef pmap<int, Colorf> ColorSheetMap;

  int count_vert_edges(const EdgePtrs &edges) const;
  plist<EggMesherStrip> &choose_strip_list(const EggMesherStrip &strip);

  void build_sheets();
  void find_fans();
  void make_quads();
  void mesh_list(Strips &strips);
  static void make_random_color(Colorf &color);

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
