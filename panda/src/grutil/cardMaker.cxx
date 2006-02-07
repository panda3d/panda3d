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
#include "transformState.h"
#include "colorAttrib.h"
#include "sceneGraphReducer.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"
#include "geomVertexFormat.h"

////////////////////////////////////////////////////////////////////
//     Function: CardMaker::reset
//       Access: Public
//  Description: Resets all the parameters to their initial defaults.
////////////////////////////////////////////////////////////////////
void CardMaker::
reset() {
  _has_uvs = true;

  _ll_pos.set(0.0f, 0.0f, 0.0f);
  _lr_pos.set(1.0f, 0.0f, 0.0f);
  _ur_pos.set(1.0f, 0.0f, 1.0f);
  _ul_pos.set(0.0f, 0.0f, 1.0f);

  _ll_tex.set(0.0f, 0.0f, 0.0f);
  _lr_tex.set(1.0f, 0.0f, 0.0f);
  _ur_tex.set(1.0f, 1.0f, 0.0f);
  _ul_tex.set(0.0f, 1.0f, 0.0f);

  _has_color = false;
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);

  _has_normals = true;
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

  CPT(GeomVertexFormat) format;
  if (_has_normals) {
    if (_has_uvs) {
      format = GeomVertexFormat::register_format(new GeomVertexArrayFormat
                                                 (InternalName::get_vertex(), 3,
                                                  GeomEnums::NT_float32, GeomEnums::C_point,
                                                  InternalName::get_normal(), 3,
                                                  GeomEnums::NT_float32, GeomEnums::C_vector,
                                                  InternalName::get_color(), 1,
                                                  GeomEnums::NT_packed_dabc, GeomEnums::C_color,
                                                  InternalName::get_texcoord(), 3,
                                                  GeomEnums::NT_float32, GeomEnums::C_texcoord));
    } else {
      format = GeomVertexFormat::get_v3n3cp();
    }
  } else {
    if (_has_uvs) {
      format = GeomVertexFormat::register_format(new GeomVertexArrayFormat
                                                 (InternalName::get_vertex(), 3,
                                                  GeomEnums::NT_float32, GeomEnums::C_point,
                                                  InternalName::get_color(), 1,
                                                  GeomEnums::NT_packed_dabc, GeomEnums::C_color,
                                                  InternalName::get_texcoord(), 3,
                                                  GeomEnums::NT_float32, GeomEnums::C_texcoord));
    } else {
      format = GeomVertexFormat::get_v3cp();
    }
  }
  
  PT(GeomVertexData) vdata = new GeomVertexData
    ("card", format, Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter color(vdata, InternalName::get_color());
  
  vertex.add_data3f(_ul_pos);
  vertex.add_data3f(_ll_pos);
  vertex.add_data3f(_ur_pos);
  vertex.add_data3f(_lr_pos);
  
  color.add_data4f(_color);
  color.add_data4f(_color);
  color.add_data4f(_color);
  color.add_data4f(_color);
  
  if (_has_uvs) {
    GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
    texcoord.add_data3f(_ul_tex);
    texcoord.add_data3f(_ll_tex);
    texcoord.add_data3f(_ur_tex);
    texcoord.add_data3f(_lr_tex);
  }
  
  if (_has_normals) {
    GeomVertexWriter normal(vdata, InternalName::get_normal());
    LVector3f n;
    n = (_ll_pos - _ul_pos).cross(_ur_pos - _ul_pos);
    n.normalize();
    normal.add_data3f(n);
    n = (_lr_pos - _ll_pos).cross(_ul_pos - _ll_pos);
    n.normalize();
    normal.add_data3f(n);
    n = (_ul_pos - _ur_pos).cross(_lr_pos - _ur_pos);
    n.normalize();
    normal.add_data3f(n);
    n = (_ur_pos - _lr_pos).cross(_ll_pos - _lr_pos);
    n.normalize();
    normal.add_data3f(n);
  }
  
  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  strip->set_shade_model(Geom::SM_uniform);
  strip->add_next_vertices(4);
  strip->close_primitive();
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);
  
  gnode->add_geom(geom);
  
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
  LVector3f frame_max = _ll_pos.fmax(_lr_pos.fmax(_ur_pos.fmax(_ul_pos)));
  LVector3f frame_min = _ll_pos.fmin(_lr_pos.fmin(_ur_pos.fmax(_ul_pos)));
  LVector3f frame_ctr = (frame_max + frame_min) * 0.5f;
  
  LVector3f geom_center((_source_frame[0] + _source_frame[1]) * 0.5f,
                        frame_ctr[1],
                        (_source_frame[2] + _source_frame[3]) * 0.5f);

  LVector3f scale((frame_max[0] - frame_min[0]) / (_source_frame[1] - _source_frame[0]),
                  0.0,
                  (frame_max[2] - frame_min[2]) / (_source_frame[3] - _source_frame[2]));
  
  LVector3f trans = frame_ctr - geom_center;

  CPT(TransformState) transform = 
    TransformState::make_pos_hpr_scale(trans, LPoint3f(0.0f, 0.0f, 0.0f), scale);
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
