// Filename: cardMaker.cxx
// Created by:  drose (16Mar02)
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

#include "cardMaker.h"
#include "geomNode.h"
#include "geomTristrip.h"
#include "transformState.h"
#include "colorAttrib.h"
#include "sceneGraphReducer.h"
#include "qpgeom.h"
#include "qpgeomTristrips.h"
#include "qpgeomVertexWriter.h"


////////////////////////////////////////////////////////////////////
//     Function: CardMaker::reset
//       Access: Public
//  Description: Resets all the parameters to their initial defaults.
////////////////////////////////////////////////////////////////////
void CardMaker::
reset() {
  _has_uvs = true;
  _ll.set(0.0f, 0.0f);
  _ur.set(1.0f, 1.0f);
  _frame.set(0.0f, 1.0f, 0.0f, 1.0f);
  _has_color = false;
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _source_geometry = (PandaNode *)NULL;
  _source_frame.set(0.0f, 0.0f, 0.0f, 0.0f);
}


////////////////////////////////////////////////////////////////////
//     Function: CardMaker::generate
//       Access: Public
//  Description: Generates a GeomNode that renders the specified
//               geometry.
////////////////////////////////////////////////////////////////////
PT(PandaNode) CardMaker::
generate() {
  if (_source_geometry != (PandaNode *)NULL) {
    return rescale_source_geometry();
  }

  PT(GeomNode) gnode = new GeomNode(get_name());

  float left = _frame[0];
  float right = _frame[1];
  float bottom = _frame[2];
  float top = _frame[3];

  if (use_qpgeom) {
    CPT(qpGeomVertexFormat) format;
    if (_has_uvs) {
      format = qpGeomVertexFormat::get_v3cpt2();
    } else {
      format = qpGeomVertexFormat::get_v3cp();
    }

    PT(qpGeomVertexData) vdata = new qpGeomVertexData
      ("card", format, qpGeomUsageHint::UH_static);
    qpGeomVertexWriter vertex(vdata, InternalName::get_vertex());
    qpGeomVertexWriter color(vdata, InternalName::get_color());

    vertex.add_data3f(Vertexf::rfu(left, 0.0f, top));
    vertex.add_data3f(Vertexf::rfu(left, 0.0f, bottom));
    vertex.add_data3f(Vertexf::rfu(right, 0.0f, top));
    vertex.add_data3f(Vertexf::rfu(right, 0.0f, bottom));

    color.add_data4f(_color);
    color.add_data4f(_color);
    color.add_data4f(_color);
    color.add_data4f(_color);

    if (_has_uvs) {
      qpGeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
      texcoord.add_data2f(_ll[0], _ur[1]);
      texcoord.add_data2f(_ll[0], _ll[1]);
      texcoord.add_data2f(_ur[0], _ur[1]);
      texcoord.add_data2f(_ur[0], _ll[1]);
    }

    PT(qpGeomTristrips) strip = new qpGeomTristrips(qpGeomUsageHint::UH_static);
    strip->add_next_vertices(4);

    PT(qpGeom) geom = new qpGeom;
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);

    gnode->add_geom(geom);

  } else {
    Geom *geom = new GeomTristrip;
    gnode->add_geom(geom);

    PTA_int lengths = PTA_int::empty_array(0);
    lengths.push_back(4);
    
    PTA_Vertexf verts;
    verts.push_back(Vertexf::rfu(left, 0.0f, top));
    verts.push_back(Vertexf::rfu(left, 0.0f, bottom));
    verts.push_back(Vertexf::rfu(right, 0.0f, top));
    verts.push_back(Vertexf::rfu(right, 0.0f, bottom));
    
    geom->set_num_prims(1);
    geom->set_lengths(lengths);
    
    geom->set_coords(verts);
    
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
  }
  
  return gnode.p();
}

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::rescale_source_geometry
//       Access: Private
//  Description: Generates the card by rescaling the source geometry
//               appropriately.
////////////////////////////////////////////////////////////////////
PT(PandaNode) CardMaker::
rescale_source_geometry() {
  PT(PandaNode) root = _source_geometry->copy_subgraph();

  // Determine the translate and scale appropriate for our geometry.
  float geom_center_x = (_source_frame[0] + _source_frame[1]) * 0.5f;
  float geom_center_y = (_source_frame[2] + _source_frame[3]) * 0.5f;

  float frame_center_x = (_frame[0] + _frame[1]) * 0.5f;
  float frame_center_y = (_frame[2] + _frame[3]) * 0.5f;

  float scale_x = 
    (_frame[1] - _frame[0]) / (_source_frame[1] - _source_frame[0]);
  float scale_y = 
    (_frame[3] - _frame[2]) / (_source_frame[3] - _source_frame[2]);

  LVector3f trans = LVector3f::rfu(frame_center_x - geom_center_x, 0.0f, 
                                   frame_center_y - geom_center_y);
  LVector3f scale = LVector3f::rfu(scale_x, 1.0f, scale_y);

  CPT(TransformState) transform = 
    TransformState::make_pos_hpr_scale(trans, LPoint3f(0.0f, 0.0f, 0.0f),
                                       scale);
  root->set_transform(transform);

  if (_has_color) {
    root->set_attrib(ColorAttrib::make_flat(_color));
  }

  // Now flatten out the geometry as much as we can.
  SceneGraphReducer reducer;
  reducer.apply_attribs(root);
  reducer.flatten(root, ~0);

  return root;
}
