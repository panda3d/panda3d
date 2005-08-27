// Filename: pgFrameStyle.cxx
// Created by:  drose (03Jul01)
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

#include "pgFrameStyle.h"
#include "geomNode.h"
#include "pandaNode.h"
#include "transparencyAttrib.h"
#include "pointerTo.h"
#include "nodePath.h"
#include "textureAttrib.h"
#include "renderState.h"
#include "shadeModelAttrib.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"

// Specifies the UV range of textures applied to the frame.  Maybe
// we'll have a reason to make this a parameter of the frame style one
// day, but for now it's hardcoded to fit the entire texture over the
// rectangular frame.
static const LVecBase4f uv_range = LVecBase4f(0.0f, 1.0f, 0.0f, 1.0f);

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

  case PGFrameStyle::T_groove:
    return out << "groove";

  case PGFrameStyle::T_ridge:
    return out << "ridge";
  }

  return out << "**unknown(" << (int)type << ")**";
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::get_internal_frame
//       Access: Published
//  Description: Computes the size of the internal frame, given the
//               indicated external frame, appropriate for this kind
//               of frame style.  This simply subtracts the border
//               width for those frame styles that include a border.
////////////////////////////////////////////////////////////////////
LVecBase4f PGFrameStyle::
get_internal_frame(const LVecBase4f &frame) const {
  LPoint2f center((frame[0] + frame[1]) / 2.0f,
                  (frame[2] + frame[3]) / 2.0f);
  LVecBase4f scaled_frame
    ((frame[0] - center[0]) * _visible_scale[0] + center[0],
     (frame[1] - center[0]) * _visible_scale[0] + center[0],
     (frame[2] - center[1]) * _visible_scale[1] + center[1],
     (frame[3] - center[1]) * _visible_scale[1] + center[1]);

  switch (_type) {
  case T_none:
  case T_flat:
    return scaled_frame;

  default:
    break;
  }

  return LVecBase4f(scaled_frame[0] + _width[0],
                    scaled_frame[1] - _width[0],
                    scaled_frame[2] + _width[1],
                    scaled_frame[3] - _width[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PGFrameStyle::
output(ostream &out) const {
  out << _type << " color = " << _color << " width = " << _width;
  if (_visible_scale != LVecBase2f(1.0f, 1.0f)) {
    out << "visible_scale = " << get_visible_scale();
  }
  if (has_texture()) {
    out << " texture = " << *get_texture();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::xform
//       Access: Public
//  Description: Applies the indicated transform to the FrameStyle.
//               The return value is true if the frame style is
//               transformed, or false if it was not affected by the
//               transform.
////////////////////////////////////////////////////////////////////
bool PGFrameStyle::
xform(const LMatrix4f &mat) {
  // All we can do is scale the X and Y bevel sizes.

  // Extract the X and Z axes from the matrix.
  LVector3f x, z;
  mat.get_row3(x, 0);
  float x_scale = x.length();
  
  mat.get_row3(z, 2);
  float z_scale = z.length();

  _width[0] *= x_scale;
  _width[1] *= z_scale;

  switch (_type) {
  case T_none:
  case T_flat:
    return false;

  case T_bevel_out:
  case T_bevel_in:
  case T_groove:
  case T_ridge:
    return true;
  }

  // Shouldn't get here, but this makes the compiler happy.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::generate_into
//       Access: Public
//  Description: Generates geometry representing a frame of the
//               indicated size, and parents it to the indicated node,
//               with a scene graph sort order of -1.
//
//               The return value is the generated NodePath, if any,
//               or an empty NodePath if nothing is generated.
////////////////////////////////////////////////////////////////////
NodePath PGFrameStyle::
generate_into(const NodePath &parent, const LVecBase4f &frame) {
  PT(PandaNode) new_node;

  LPoint2f center((frame[0] + frame[1]) / 2.0f,
                  (frame[2] + frame[3]) / 2.0f);
  LVecBase4f scaled_frame
    ((frame[0] - center[0]) * _visible_scale[0] + center[0],
     (frame[1] - center[0]) * _visible_scale[0] + center[0],
     (frame[2] - center[1]) * _visible_scale[1] + center[1],
     (frame[3] - center[1]) * _visible_scale[1] + center[1]);

  switch (_type) {
  case T_none:
    return NodePath();

  case T_flat:
    new_node = generate_flat_geom(scaled_frame);
    break;

  case T_bevel_out:
    new_node = generate_bevel_geom(scaled_frame, false);
    break;

  case T_bevel_in:
    new_node = generate_bevel_geom(scaled_frame, true);
    break;

  case T_groove:
    new_node = generate_groove_geom(scaled_frame, true);
    break;

  case T_ridge:
    new_node = generate_groove_geom(scaled_frame, false);
    break;

  default:
    break;
  }

  if (new_node != (PandaNode *)NULL && _color[3] != 1.0f) {
    // We've got some alpha on the color; we need transparency.
    new_node->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  // Adding the node to the parent keeps the reference count.
  return parent.attach_new_node(new_node);
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::generate_flat_geom
//       Access: Private
//  Description: Generates the GeomNode appropriate to a T_flat
//               frame.
////////////////////////////////////////////////////////////////////
PT(PandaNode) PGFrameStyle::
generate_flat_geom(const LVecBase4f &frame) {
  PT(GeomNode) gnode = new GeomNode("flat");

  float left = frame[0];
  float right = frame[1];
  float bottom = frame[2];
  float top = frame[3];

  CPT(GeomVertexFormat) format;
  if (has_texture()) {
    format = GeomVertexFormat::get_v3cpt2();
  } else {
    format = GeomVertexFormat::get_v3cp();
  }
  
  PT(GeomVertexData) vdata = new GeomVertexData
    ("PGFrame", format, Geom::UH_static);
  
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter color(vdata, InternalName::get_color());
  vertex.add_data3f(left, 0.0f, top);
  vertex.add_data3f(left, 0.0f, bottom);
  vertex.add_data3f(right, 0.0f, top);
  vertex.add_data3f(right, 0.0f, bottom);
  
  color.add_data4f(_color);
  color.add_data4f(_color);
  color.add_data4f(_color);
  color.add_data4f(_color);

  if (has_texture()) {
    // Generate UV's.
    left = uv_range[0];
    right = uv_range[1];
    bottom = uv_range[2];
    top = uv_range[3];
    
    GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
    texcoord.add_data2f(left, top);
    texcoord.add_data2f(left, bottom);
    texcoord.add_data2f(right, top);
    texcoord.add_data2f(right, bottom);
  }
  
  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  strip->add_next_vertices(4);
  strip->close_primitive();
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);
  gnode->add_geom(geom);
  
  if (has_texture()) {
    CPT(RenderState) state = 
      RenderState::make(TextureAttrib::make(get_texture()));
    gnode->set_geom_state(0, state);
  }
  
  return gnode.p();
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::generate_bevel_geom
//       Access: Private
//  Description: Generates the GeomNode appropriate to a T_bevel_in or
//               T_bevel_out frame.
////////////////////////////////////////////////////////////////////
PT(PandaNode) PGFrameStyle::
generate_bevel_geom(const LVecBase4f &frame, bool in) {
  //
  // Colors:
  //
  // 
  //  * * * * * * * * * * * * * * * * * * * * * * *
  //  * *                                       * *
  //  *   *               ctop                *   *
  //  *     *                               *     *
  //  *       * * * * * * * * * * * * * * *       *
  //  *       *                           *       *
  //  *       *                           *       *
  //  * cleft *          _color           * cright*
  //  *       *                           *       *
  //  *       *                           *       *
  //  *       * * * * * * * * * * * * * * *       *
  //  *     *                               *     *
  //  *   *              cbottom              *   *
  //  * *                                       * *
  //  * * * * * * * * * * * * * * * * * * * * * * *
  //
  //
  // Vertices:
  //
  //  tristrip 1:
  //  4 * * * * * * * * * * * * * * * * * * * * * 6
  //  * *                                       *
  //  *   *                                   *
  //  *     *                               *
  //  *       5 * * * * * * * * * * * * * 7
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       3 * * * * * * * * * * * * * 1
  //  *     *                               *
  //  *   *                                   *
  //  * *                                       *
  //  2 * * * * * * * * * * * * * * * * * * * * * 0
  // 
  //  tristrip 2:
  //                                              1
  //                                            * *
  //                                          *   *
  //                                        *     *
  //          5 * * * * * * * * * * * * * 3       *
  //          *                           *       *
  //          *                           *       *
  //          *                           *       *
  //          *                           *       *
  //          *                           *       *
  //          4 * * * * * * * * * * * * * 2       *
  //                                        *     *
  //                                          *   *
  //                                            * *
  //                                              0

  PT(GeomNode) gnode = new GeomNode("bevel");

  float left = frame[0];
  float right = frame[1];
  float bottom = frame[2];
  float top = frame[3];

  float inner_left = left + _width[0];
  float inner_right = right - _width[0];
  float inner_bottom = bottom + _width[1];
  float inner_top = top - _width[1];

  float left_color_scale = 1.2;
  float right_color_scale = 0.8;
  float bottom_color_scale = 0.7;
  float top_color_scale = 1.3;

  if (in) {
    right_color_scale = 1.2;
    left_color_scale = 0.8;
    top_color_scale = 0.7;
    bottom_color_scale = 1.3;
  }

  // Clamp all colors at white, and don't scale the alpha.
  Colorf cleft(min(_color[0] * left_color_scale, 1.0f),
               min(_color[1] * left_color_scale, 1.0f),
               min(_color[2] * left_color_scale, 1.0f),
               _color[3]);

  Colorf cright(min(_color[0] * right_color_scale, 1.0f),
                min(_color[1] * right_color_scale, 1.0f),
                min(_color[2] * right_color_scale, 1.0f),
                _color[3]);

  Colorf cbottom(min(_color[0] * bottom_color_scale, 1.0f),
                 min(_color[1] * bottom_color_scale, 1.0f),
                 min(_color[2] * bottom_color_scale, 1.0f),
                 _color[3]);

  Colorf ctop(min(_color[0] * top_color_scale, 1.0f),
              min(_color[1] * top_color_scale, 1.0f),
              min(_color[2] * top_color_scale, 1.0f),
              _color[3]);

  CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3cp();
  PT(GeomVertexData) vdata = new GeomVertexData
    ("PGFrame", format, Geom::UH_static);
  
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter color(vdata, InternalName::get_color());
  
  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  // Tristrip 1.
  vertex.add_data3f(right, 0.0f, bottom);
  vertex.add_data3f(inner_right, 0.0f, inner_bottom);
  vertex.add_data3f(left, 0.0f, bottom);
  vertex.add_data3f(inner_left, 0.0f, inner_bottom);
  vertex.add_data3f(left, 0.0f, top);
  vertex.add_data3f(inner_left, 0.0f, inner_top);
  vertex.add_data3f(right, 0.0f, top);
  vertex.add_data3f(inner_right, 0.0f, inner_top);
  color.add_data4f(cbottom);
  color.add_data4f(cbottom);
  color.add_data4f(cbottom);
  color.add_data4f(cbottom);
  color.add_data4f(cleft);
  color.add_data4f(cleft);
  color.add_data4f(ctop);
  color.add_data4f(ctop);
  
  strip->add_next_vertices(8);
  strip->close_primitive();
  
  // Tristrip 2.
  vertex.add_data3f(right, 0.0f, bottom);
  vertex.add_data3f(right, 0.0f, top);
  vertex.add_data3f(inner_right, 0.0f, inner_bottom);
  vertex.add_data3f(inner_right, 0.0f, inner_top);
  vertex.add_data3f(inner_left, 0.0f, inner_bottom);
  vertex.add_data3f(inner_left, 0.0f, inner_top);
  color.add_data4f(cright);
  color.add_data4f(cright);
  color.add_data4f(cright);
  color.add_data4f(cright);
  color.add_data4f(_color);
  color.add_data4f(_color);
  
  strip->add_next_vertices(6);
  strip->close_primitive();
  strip->set_shade_model(Geom::SM_flat_last_vertex);
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);
  
  CPT(RenderState) flat_state = RenderState::make(ShadeModelAttrib::make(ShadeModelAttrib::M_flat));
  gnode->add_geom(geom, flat_state);
  
  // For now, beveled and grooved geoms don't support textures.  Easy
  // to add if anyone really wants this.
  
  return gnode.p();
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::generate_groove_geom
//       Access: Private
//  Description: Generates the GeomNode appropriate to a T_groove or
//               T_ridge frame.
////////////////////////////////////////////////////////////////////
PT(PandaNode) PGFrameStyle::
generate_groove_geom(const LVecBase4f &frame, bool in) {
  //
  // Colors:
  //
  // 
  //  * * * * * * * * * * * * * * * * * * * * * * * * * * *
  //  * *                                               * *
  //  *   *                   ctop                    *   *
  //  *     *                                       *     *
  //  *       * * * * * * * * * * * * * * * * * * *       *
  //  *       * *                               * *       *
  //  *       *   *          cbottom          *   *       *
  //  *       *     *                       *     *       *
  //  *       *       * * * * * * * * * * *       *       *
  //  *       *       *                   *       *       *
  //  *       *       *                   *       *       *
  //  * cleft * cright*      _color       * cleft * cright*
  //  *       *       *                   *       *       *
  //  *       *       *                   *       *       *
  //  *       *       * * * * * * * * * * *       *       *
  //  *       *     *                       *     *       *
  //  *       *   *           ctop            *   *       *
  //  *       * *                               * *       *
  //  *       * * * * * * * * * * * * * * * * * * *       *
  //  *     *                                       *     *
  //  *   *                  cbottom                  *   *
  //  * *                                               * *
  //  * * * * * * * * * * * * * * * * * * * * * * * * * * *
  //
  //
  // Vertices:
  //
  //  tristrip 1:
  //  4 * * * * * * * * * * * * * * * * * * * * * * * * * 6
  //  * *                                               *
  //  *   *                                           *
  //  *     *                                       *
  //  *       5 * * * * * * * * * * * * * * * * * 7
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       *
  //  *       3 * * * * * * * * * * * * * * * * * 1
  //  *     *                                       *
  //  *   *                                           *
  //  * *                                               *
  //  2 * * * * * * * * * * * * * * * * * * * * * * * * * 0
  //
  //  tristrip 2:
  //          4 * * * * * * * * * * * * * * * * * 6
  //          * *                               *
  //          *   *                           *
  //          *     *                       *
  //          *       5 * * * * * * * * * 7
  //          *       *
  //          *       *
  //          *       *
  //          *       *
  //          *       *
  //          *       3 * * * * * * * * * 1
  //          *     *                       *
  //          *   *                           *
  //          * *                               *
  //          2 * * * * * * * * * * * * * * * * * 0
  // 
  //  tristrip 3:
  //                                                      1
  //                                                    * *
  //                                                  *   *
  //                                                *     *
  //                                              3       *
  //                                            * *       *
  //                                          *   *       *
  //                                        *     *       *
  //                  7 * * * * * * * * * 5       *       *
  //                  *                   *       *       *
  //                  *                   *       *       *
  //                  *                   *       *       *
  //                  *                   *       *       *
  //                  *                   *       *       *
  //                  6 * * * * * * * * * 4       *       *
  //                                        *     *       *
  //                                          *   *       *
  //                                            * *       *
  //                                              2       *
  //                                                *     *
  //                                                  *   *
  //                                                    * *
  //                                                      0

  PT(GeomNode) gnode = new GeomNode("groove");

  float left = frame[0];
  float right = frame[1];
  float bottom = frame[2];
  float top = frame[3];

  float mid_left = left + 0.5f * _width[0];
  float mid_right = right - 0.5f * _width[0];
  float mid_bottom = bottom + 0.5f * _width[1];
  float mid_top = top - 0.5f * _width[1];

  float inner_left = left + _width[0];
  float inner_right = right - _width[0];
  float inner_bottom = bottom + _width[1];
  float inner_top = top - _width[1];

  float left_color_scale = 1.2f;
  float right_color_scale = 0.8f;
  float bottom_color_scale = 0.7f;
  float top_color_scale = 1.3f;

  if (in) {
    right_color_scale = 1.2f;
    left_color_scale = 0.8f;
    top_color_scale = 0.7f;
    bottom_color_scale = 1.3f;
  }

  // Clamp all colors at white, and don't scale the alpha.
  Colorf cleft(min(_color[0] * left_color_scale, 1.0f),
               min(_color[1] * left_color_scale, 1.0f),
               min(_color[2] * left_color_scale, 1.0f),
               _color[3]);

  Colorf cright(min(_color[0] * right_color_scale, 1.0f),
                min(_color[1] * right_color_scale, 1.0f),
                min(_color[2] * right_color_scale, 1.0f),
                _color[3]);

  Colorf cbottom(min(_color[0] * bottom_color_scale, 1.0f),
                 min(_color[1] * bottom_color_scale, 1.0f),
                 min(_color[2] * bottom_color_scale, 1.0f),
                 _color[3]);

  Colorf ctop(min(_color[0] * top_color_scale, 1.0f),
              min(_color[1] * top_color_scale, 1.0f),
              min(_color[2] * top_color_scale, 1.0f),
              _color[3]);

  CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3cp();
  PT(GeomVertexData) vdata = new GeomVertexData
    ("PGFrame", format, Geom::UH_static);
  
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter color(vdata, InternalName::get_color());
  
  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  // Tristrip 1.
  vertex.add_data3f(right, 0.0f, bottom);
  vertex.add_data3f(mid_right, 0.0f, mid_bottom);
  vertex.add_data3f(left, 0.0f, bottom);
  vertex.add_data3f(mid_left, 0.0f, mid_bottom);
  vertex.add_data3f(left, 0.0f, top);
  vertex.add_data3f(mid_left, 0.0f, mid_top);
  vertex.add_data3f(right, 0.0f, top);
  vertex.add_data3f(mid_right, 0.0f, mid_top);
  color.add_data4f(cbottom);
  color.add_data4f(cbottom);
  color.add_data4f(cbottom);
  color.add_data4f(cbottom);
  color.add_data4f(cleft);
  color.add_data4f(cleft);
  color.add_data4f(ctop);
  color.add_data4f(ctop);
  
  strip->add_next_vertices(8);
  strip->close_primitive();
  
  // Tristrip 2.
  vertex.add_data3f(mid_right, 0.0f, mid_bottom);
  vertex.add_data3f(inner_right, 0.0f, inner_bottom);
  vertex.add_data3f(mid_left, 0.0f, mid_bottom);
  vertex.add_data3f(inner_left, 0.0f, inner_bottom);
  vertex.add_data3f(mid_left, 0.0f, mid_top);
  vertex.add_data3f(inner_left, 0.0f, inner_top);
  vertex.add_data3f(mid_right, 0.0f, mid_top);
  vertex.add_data3f(inner_right, 0.0f, inner_top);
  color.add_data4f(ctop);
  color.add_data4f(ctop);
  color.add_data4f(ctop);
  color.add_data4f(ctop);
  color.add_data4f(cright);
  color.add_data4f(cright);
  color.add_data4f(cbottom);
  color.add_data4f(cbottom);
  
  strip->add_next_vertices(8);
  strip->close_primitive();
  
  // Tristrip 3.
  vertex.add_data3f(right, 0.0f, bottom);
  vertex.add_data3f(right, 0.0f, top);
  vertex.add_data3f(mid_right, 0.0f, mid_bottom);
  vertex.add_data3f(mid_right, 0.0f, mid_top);
  vertex.add_data3f(inner_right, 0.0f, inner_bottom);
  vertex.add_data3f(inner_right, 0.0f, inner_top);
  vertex.add_data3f(inner_left, 0.0f, inner_bottom);
  vertex.add_data3f(inner_left, 0.0f, inner_top);
  color.add_data4f(cright);
  color.add_data4f(cright);
  color.add_data4f(cright);
  color.add_data4f(cright);
  color.add_data4f(cleft);
  color.add_data4f(cleft);
  color.add_data4f(_color);
  color.add_data4f(_color);
  
  strip->add_next_vertices(8);
  strip->close_primitive();
  
  strip->set_shade_model(Geom::SM_flat_last_vertex);
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);
  
  CPT(RenderState) flat_state = RenderState::make(ShadeModelAttrib::make(ShadeModelAttrib::M_flat));
  gnode->add_geom(geom, flat_state);
  
  // For now, beveled and grooved geoms don't support textures.  Easy
  // to add if anyone really wants this.
  
  return gnode.p();
}
