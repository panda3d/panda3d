// Filename: cardMaker.cxx
// Created by:  drose (06Jul01)
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

#include "cardMaker.h"
#include "geomNode.h"
#include "geomTristrip.h"


////////////////////////////////////////////////////////////////////
//     Function: CardMaker::reset
//       Access: Public
//  Description: Resets all the parameters to their initial defaults.
////////////////////////////////////////////////////////////////////
void CardMaker::
reset() {
  _has_uvs = true;
  _ll.set(0.0, 0.0);
  _ur.set(1.0, 1.0);
  _frame.set(0.0, 1.0, 0.0, 1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: CardMaker::generate
//       Access: Public
//  Description: Generates a GeomNode that renders the specified
//               geometry.
////////////////////////////////////////////////////////////////////
PT_Node CardMaker::
generate() {
  PT(GeomNode) gnode = new GeomNode("card");
  Geom *geom = new GeomTristrip;
  gnode->add_geom(geom);

  float left = _frame[0];
  float right = _frame[1];
  float bottom = _frame[2];
  float top = _frame[3];

  PTA_int lengths(0);
  lengths.push_back(4);

  PTA_Vertexf verts;
  verts.push_back(Vertexf::rfu(left, 0.0, top));
  verts.push_back(Vertexf::rfu(left, 0.0, bottom));
  verts.push_back(Vertexf::rfu(right, 0.0, top));
  verts.push_back(Vertexf::rfu(right, 0.0, bottom));

  geom->set_num_prims(1);
  geom->set_lengths(lengths);

  geom->set_coords(verts, G_PER_VERTEX);

  PTA_Colorf colors;
  colors.push_back(Colorf(1.0, 1.0, 1.0, 1.0));
  geom->set_colors(colors, G_OVERALL);

  if (_has_uvs) {
    PTA_TexCoordf uvs;
    uvs.push_back(TexCoordf(_ll[0], _ur[1]));
    uvs.push_back(TexCoordf(_ll[0], _ll[1]));
    uvs.push_back(TexCoordf(_ur[0], _ur[1]));
    uvs.push_back(TexCoordf(_ur[0], _ll[1]));
    geom->set_texcoords(uvs, G_PER_VERTEX);
  }
  
  return gnode.p();
}
