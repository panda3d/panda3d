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
#include "renderRelation.h"
#include "transformTransition.h"
#include "colorTransition.h"
#include "sceneGraphReducer.h"


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
  _has_color = false;
  _color.set(1.0, 1.0, 1.0, 1.0);
  _source_geometry = (Node *)NULL;
  _source_frame.set(0.0, 0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////////////////////
//     Function: CardMaker::generate
//       Access: Public
//  Description: Generates a GeomNode that renders the specified
//               geometry.
////////////////////////////////////////////////////////////////////
PT_Node CardMaker::
generate() {
  if (_source_geometry != (Node *)NULL) {
    return rescale_source_geometry();
  }

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
  colors.push_back(_color);
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

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::rescale_source_geometry
//       Access: Private
//  Description: Generates the card by rescaling the source geometry
//               appropriately.
////////////////////////////////////////////////////////////////////
PT_Node CardMaker::
rescale_source_geometry() {
  PT_Node root = new NamedNode;
  NodeRelation *root_arc = 
    new RenderRelation(root, _source_geometry->copy_subgraph(RenderRelation::get_class_type()));

  // Determine the translate and scale appropriate for our geometry.
  float geom_center_x = (_source_frame[0] + _source_frame[1]) / 2.0;
  float geom_center_y = (_source_frame[2] + _source_frame[3]) / 2.0;

  float frame_center_x = (_frame[0] + _frame[1]) / 2.0;
  float frame_center_y = (_frame[2] + _frame[3]) / 2.0;

  float scale_x = 
    (_frame[1] - _frame[0]) / (_source_frame[1] - _source_frame[0]);
  float scale_y = 
    (_frame[3] - _frame[2]) / (_source_frame[3] - _source_frame[2]);

  LVector3f trans = LVector3f::rfu(frame_center_x - geom_center_x, 0.0, 
                                   frame_center_y - geom_center_y);
  LVector3f scale = LVector3f::rfu(scale_x, 1.0, scale_y);

  LMatrix4f mat = 
    LMatrix4f::scale_mat(scale) *
    LMatrix4f::translate_mat(trans);

  root_arc->set_transition(new TransformTransition(mat));

  if (_has_color) {
    root_arc->set_transition(new ColorTransition(_color));
  }

  // Now flatten out the geometry as much as we can.
  SceneGraphReducer reducer;
  reducer.apply_transitions(root_arc);
  reducer.flatten(root, true);

  if (root->get_num_children(RenderRelation::get_class_type()) == 1) {
    // If we ended up with only one child, and no transitions on the
    // arc to it, return that child.
    NodeRelation *arc = root->get_child(RenderRelation::get_class_type(), 0);
    if (!arc->has_any_transition()) {
      return arc->get_child();
    }
  }

  return root;
}
