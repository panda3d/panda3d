/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cardMaker.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "cardMaker.h"
#include "geomNode.h"
#include "transformState.h"
#include "colorAttrib.h"
#include "sceneGraphReducer.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"
#include "geomVertexFormat.h"

/**
 * Resets all the parameters to their initial defaults.
 */
void CardMaker::
reset() {
  set_frame(0.0f, 1.0f, 0.0f, 1.0f);
  set_uv_range(LTexCoord(0.0f, 0.0f), LTexCoord(1.0f, 1.0f));

  _has_color = false;
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);

  _has_normals = true;
  _source_geometry = nullptr;
  _source_frame.set(0.0f, 0.0f, 0.0f, 0.0f);
}


/**
 * Generates a GeomNode that renders the specified geometry.
 */
PT(PandaNode) CardMaker::
generate() {
  if (_source_geometry != nullptr) {
    return rescale_source_geometry();
  }

  PT(GeomNode) gnode = new GeomNode(get_name());

  CPT(GeomVertexFormat) format;
  if (_has_normals) {
    if (_has_uvs) {
      if (_has_3d_uvs) {
        format = GeomVertexFormat::register_format
          (new GeomVertexArrayFormat
           (InternalName::get_vertex(), 3,
            GeomEnums::NT_stdfloat, GeomEnums::C_point,
            InternalName::get_normal(), 3,
            GeomEnums::NT_stdfloat, GeomEnums::C_normal,
            InternalName::get_texcoord(), 3,
            GeomEnums::NT_stdfloat, GeomEnums::C_texcoord));
      } else {
        format = GeomVertexFormat::get_v3n3t2();
      }
    } else {
      format = GeomVertexFormat::get_v3n3();
    }
  } else {
    if (_has_uvs) {
      if (_has_3d_uvs) {
        format = GeomVertexFormat::register_format
          (new GeomVertexArrayFormat
           (InternalName::get_vertex(), 3,
            GeomEnums::NT_stdfloat, GeomEnums::C_point,
            InternalName::get_texcoord(), 3,
            GeomEnums::NT_stdfloat, GeomEnums::C_texcoord));
      } else {
        format = GeomVertexFormat::get_v3t2();
      }
    } else {
      format = GeomVertexFormat::get_v3();
    }
  }

  PT(GeomVertexData) vdata = new GeomVertexData
    ("card", format, Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());

  vertex.add_data3(_ul_pos);
  vertex.add_data3(_ll_pos);
  vertex.add_data3(_ur_pos);
  vertex.add_data3(_lr_pos);

  if (_has_uvs) {
    GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
    texcoord.add_data3(_ul_tex);
    texcoord.add_data3(_ll_tex);
    texcoord.add_data3(_ur_tex);
    texcoord.add_data3(_lr_tex);
  }

  if (_has_normals) {
    GeomVertexWriter normal(vdata, InternalName::get_normal());
    LVector3 n;
    n = (_ll_pos - _ul_pos).cross(_ur_pos - _ul_pos);
    n.normalize();
    normal.add_data3(n);
    n = (_lr_pos - _ll_pos).cross(_ul_pos - _ll_pos);
    n.normalize();
    normal.add_data3(n);
    n = (_ul_pos - _ur_pos).cross(_lr_pos - _ur_pos);
    n.normalize();
    normal.add_data3(n);
    n = (_ur_pos - _lr_pos).cross(_ll_pos - _lr_pos);
    n.normalize();
    normal.add_data3(n);
  }

  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  strip->set_shade_model(Geom::SM_uniform);
  strip->add_next_vertices(4);
  strip->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);

  CPT(RenderState) state = RenderState::make_empty();
  if (_has_color) {
    state = RenderState::make(ColorAttrib::make_flat(_color));
  }

  gnode->add_geom(geom, state);

  return gnode;
}

/**
 * Sets the range of UV's that will be applied to the vertices.  If
 * set_has_uvs() is true (as it is by default), the vertices will be generated
 * with the indicated range of UV's, which will be useful if a texture is
 * applied.
 */
void CardMaker::
set_uv_range(const LTexCoord3 &ll, const LTexCoord3 &lr, const LTexCoord3 &ur, const LTexCoord3 &ul) {
  _ll_tex = ll;
  _lr_tex = lr;
  _ur_tex = ur;
  _ul_tex = ul;
  _has_uvs = true;
  _has_3d_uvs = true;
}

/**
 * Sets the range of UV's that will be applied to the vertices.  If
 * set_has_uvs() is true (as it is by default), the vertices will be generated
 * with the indicated range of UV's, which will be useful if a texture is
 * applied.
 */
void CardMaker::
set_uv_range(const LTexCoord &ll, const LTexCoord &lr, const LTexCoord &ur, const LTexCoord &ul) {
  _ll_tex.set(ll[0], ll[1], 0.0f);
  _lr_tex.set(lr[0], lr[1], 0.0f);
  _ur_tex.set(ur[0], ur[1], 0.0f);
  _ul_tex.set(ul[0], ul[1], 0.0f);
  _has_uvs = true;
  _has_3d_uvs = false;
}

/**
 * Sets the range of UV's that will be applied to the vertices.  If
 * set_has_uvs() is true (as it is by default), the vertices will be generated
 * with the indicated range of UV's, which will be useful if a texture is
 * applied.
 */
void CardMaker::
set_uv_range(const LTexCoord &ll, const LTexCoord &ur) {
  _ll_tex.set(ll[0], ll[1], 0.0f);
  _lr_tex.set(ur[0], ll[1], 0.0f);
  _ur_tex.set(ur[0], ur[1], 0.0f);
  _ul_tex.set(ll[0], ur[1], 0.0f);
  _has_uvs = true;
  _has_3d_uvs = false;
}

/**
 * Sets the range of UV's that will be applied to the vertices.  If
 * set_has_uvs() is true (as it is by default), the vertices will be generated
 * with the indicated range of UV's, which will be useful if a texture is
 * applied.
 */
void CardMaker::
set_uv_range(const LVector4 &x, const LVector4 &y, const LVector4 &z) {
  _ll_tex.set(x[0], y[0], z[0]);
  _lr_tex.set(x[1], y[1], z[1]);
  _ur_tex.set(x[2], y[2], z[2]);
  _ul_tex.set(x[3], y[3], z[3]);
  _has_uvs = true;
  _has_3d_uvs = true;
}

/**
 * Sets the range of UV's that will be applied to the vertices appropriately
 * for a cube-map face.
 */
void CardMaker::
set_uv_range_cube(int face) {
  LVector4 varya(-1,  1,  1, -1);
  LVector4 varyb(-1, -1,  1,  1);
  LVector4 fixed( 1,  1,  1,  1);
  switch(face) {
  case 0: set_uv_range( fixed, -varyb, -varya); break; // positive_x
  case 1: set_uv_range(-fixed, -varyb,  varya); break; // negative_x
  case 2: set_uv_range( varya,  fixed,  varyb); break; // positive_y
  case 3: set_uv_range( varya, -fixed, -varyb); break; // negative_y
  case 4: set_uv_range( varya, -varyb,  fixed); break; // positive_z
  case 5: set_uv_range(-varya, -varyb, -fixed); break; // negative_z
  }
}

/**
 * Sets the range of UV's that will be applied to the vertices appropriately
 * to show the non-pad region of the texture.
 */
void CardMaker::
set_uv_range(const Texture *tex) {
  nassertv(tex->get_texture_type() == Texture::TT_2d_texture);
  int nonpadx = tex->get_x_size() - tex->get_pad_x_size();
  int nonpady = tex->get_y_size() - tex->get_pad_y_size();
  double maxu = (nonpadx*1.0) / tex->get_x_size();
  double maxv = (nonpady*1.0) / tex->get_y_size();
  set_uv_range(LTexCoord(0.0,0.0), LTexCoord(maxu,maxv));
}

/**
 * Generates the card by rescaling the source geometry appropriately.
 */
PT(PandaNode) CardMaker::
rescale_source_geometry() {
  PT(PandaNode) root = _source_geometry->copy_subgraph();

  // Determine the translate and scale appropriate for our geometry.
  LVector3 frame_max = _ll_pos.fmax(_lr_pos.fmax(_ur_pos.fmax(_ul_pos)));
  LVector3 frame_min = _ll_pos.fmin(_lr_pos.fmin(_ur_pos.fmax(_ul_pos)));
  LVector3 frame_ctr = (frame_max + frame_min) * 0.5f;

  LVector3 geom_center((_source_frame[0] + _source_frame[1]) * 0.5f,
                        frame_ctr[1],
                        (_source_frame[2] + _source_frame[3]) * 0.5f);

  LVector3 scale((frame_max[0] - frame_min[0]) / (_source_frame[1] - _source_frame[0]),
                  0.0,
                  (frame_max[2] - frame_min[2]) / (_source_frame[3] - _source_frame[2]));

  LVector3 trans = frame_ctr - geom_center;

  CPT(TransformState) transform =
    TransformState::make_pos_hpr_scale(trans, LPoint3(0.0f, 0.0f, 0.0f), scale);
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
