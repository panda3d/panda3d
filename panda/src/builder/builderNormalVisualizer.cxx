// Filename: builderNormalVisualizer.cxx
// Created by:  drose (08Sep99)
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

#include "builderFuncs.h"
#include "builderNormalVisualizer.h"
#include "geomNode.h"

#ifdef SUPPORT_SHOW_NORMALS

void BuilderNormalVisualizer::
add_prim(const BuilderPrim &prim) {
  if (prim.has_overall_normal()) {
    // Average up all the vertex values to get the primitive center.
    BuilderV net_vertex;
    net_vertex.set(0.0, 0.0, 0.0);
    int num_verts = prim.get_num_verts();
    for (int i = 0; i < num_verts; i++) {
      net_vertex += prim.get_vertex(i).get_coord();
    }
    net_vertex /= num_verts;
    add_normal(net_vertex, prim.get_normal());

  } else if (prim.has_vertex_normal()) {
    // Each vertex gets its own normal.
    int num_verts = prim.get_num_verts();
    for (int i = 0; i < num_verts; i++) {
      add_normal(prim.get_vertex(i).get_coord(), prim.get_vertex(i).get_normal());
    }
  } else if (prim.has_component_normal()) {
    // Each component gets its own normal.  We don't presently handle
    // this, as it should only happen when the user passes
    // already-meshed tristrips into the builder, an unusual
    // situation.
  }
}

void BuilderNormalVisualizer::
add_prim(const BuilderPrimI &prim) {
  BuilderPrim prim_ni;
  prim_ni.nonindexed_copy(prim, _bucket);
  add_prim(prim_ni);
}

void BuilderNormalVisualizer::
show_normals(GeomNode *node) {
  // Ok, now we've got a bunch of normals saved up; create some geometry.
  mesh_and_build(_lines.begin(), _lines.end(), _bucket, node, (BuilderPrim *)0);
}

void BuilderNormalVisualizer::
add_normal(const BuilderV &center, const BuilderN &normal) {
  BuilderV to = center + normal * _bucket._normal_scale;

  BuilderPrim line;
  line.set_type(BPT_line);
  line.set_color(_bucket._normal_color);
  line.add_vertex(BuilderVertex(center));
  line.add_vertex(BuilderVertex(to));

  _lines.push_back(line);
}

#endif

