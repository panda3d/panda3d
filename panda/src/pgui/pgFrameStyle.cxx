// Filename: pgFrameStyle.cxx
// Created by:  drose (03Jul01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pgFrameStyle.h"
#include "geomTristrip.h"
#include "geomNode.h"
#include "transparencyProperty.h"
#include "transparencyTransition.h"
#include "renderRelation.h"

ostream &
operator << (ostream &out, PGFrameStyle::Type type) {
  switch (type) {
  case PGFrameStyle::T_none:
    return out << "none";

  case PGFrameStyle::T_flat:
    return out << "flat";

  case PGFrameStyle::T_bevel_out:
    return out << "bevel_out";

  case PGFrameStyle::T_bevel_in:
    return out << "bevel_in";
  }

  return out << "**unknown(" << (int)type << ")**";
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PGFrameStyle::
output(ostream &out) const {
  out << _type << " color = " << _color << " width = " << _width;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::generate_into
//       Access: Public
//  Description: Generates geometry representing a frame of the
//               indicated size, and parents it to the indicated node,
//               with a scene graph sort order of -1.
//
//               The return value is the generated arc, if any, or
//               NULL if nothing is generated.
////////////////////////////////////////////////////////////////////
NodeRelation *PGFrameStyle::
generate_into(Node *node, const LVecBase4f &frame) {
  NodeRelation *arc = (NodeRelation *)NULL;
  PT(Geom) geom;

  switch (_type) {
  case T_none:
    return (NodeRelation *)NULL;

  case T_flat:
    geom = generate_flat_geom(frame);
    break;

  default:
    break;
  }

  if (geom != (Geom *)NULL) {
    // We've got a basic Geom; create a GeomNode for it.
    PT(GeomNode) gnode = new GeomNode("frame");
    gnode->add_geom(geom);
    arc = new RenderRelation(node, gnode, -1);
  }

  if (arc != (NodeRelation *)NULL && _color[3] != 1.0) {
    // We've got some alpha on the color; we need transparency.
    TransparencyProperty::Mode mode = TransparencyProperty::M_alpha;
    TransparencyTransition *tt = new TransparencyTransition(mode);
    arc->set_transition(tt);
  }

  return arc;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::generate_flat_geom
//       Access: Public
//  Description: Generates the Geom object appropriate to a T_flat
//               frame.
////////////////////////////////////////////////////////////////////
PT(Geom) PGFrameStyle::
generate_flat_geom(const LVecBase4f &frame) {
  PT(Geom) geom = new GeomTristrip;

  float left = frame[0];
  float right = frame[1];
  float bottom = frame[2];
  float top = frame[3];

  PTA_int lengths(0);
  lengths.push_back(4);

  PTA_Vertexf verts;
  verts.push_back(Vertexf(left, 0.0, top));
  verts.push_back(Vertexf(left, 0.0, bottom));
  verts.push_back(Vertexf(right, 0.0, top));
  verts.push_back(Vertexf(right, 0.0, bottom));

  geom->set_num_prims(1);
  geom->set_lengths(lengths);

  geom->set_coords(verts, G_PER_VERTEX);

  PTA_Colorf colors;
  colors.push_back(_color);
  geom->set_colors(colors, G_OVERALL);
  
  return geom;
}
