/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToFlt.cxx
 * @author drose
 * @date 2003-10-01
 */

#include "eggToFlt.h"
#include "fltHeader.h"
#include "fltBead.h"
#include "fltGroup.h"
#include "fltFace.h"
#include "fltVertexList.h"
#include "fltVertex.h"
#include "fltTexture.h"
#include "fltTransformTranslate.h"
#include "fltTransformRotateAboutEdge.h"
#include "fltTransformScale.h"
#include "fltTransformGeneralMatrix.h"
#include "eggPolygon.h"
#include "eggPoint.h"
#include "eggPrimitive.h"
#include "eggExternalReference.h"
#include "eggGroup.h"
#include "eggGroupNode.h"
#include "eggTexture.h"
#include "eggTransform.h"
#include "dcast.h"
#include "string_utils.h"
#include "vector_string.h"

/**
 *
 */
EggToFlt::
EggToFlt() :
  EggToSomething("MultiGen", ".flt", true, false)
{
  set_binary_output(true);
  set_program_brief("convert files from .egg format to MultiGen .flt");
  set_program_description
    ("egg2lt converts files from egg format to MultiGen .flt "
     "format.  It attempts to be as robust as possible, and matches "
     "the capabilities of flt2egg.  Generally, converting a model "
     "from egg2lt and then back via flt2egg will result in essentially "
     "the same egg file, within the limitations of what can be "
     "represented in flt.");

  add_option
    ("attr", "none/new/all", 0,
     "Specifies whether to write (or rewrite) .attr files for each "
     "texture image.  MultiGen stores texture properties like mipmapping "
     "in a separate .attr file for each different texture image.  "
     "If this parameter is \"none\", these files will not be generated; "
     "if this is \"new\", these files will only be generated if they "
     "do not already exist (even if the properties have changed).  "
     "Specifying \"all\" causes these to be rewritten every time.",
     &EggToFlt::dispatch_attr, nullptr, &_auto_attr_update);

  // Flt files are always in the z-up coordinate system.  Don't confuse the
  // user with this meaningless option.
  remove_option("cs");
  _coordinate_system = CS_zup_right;
  _got_coordinate_system = true;
  _auto_attr_update = FltHeader::AU_if_missing;
}

/**
 *
 */
void EggToFlt::
run() {
  _flt_header = new FltHeader(_path_replace);
  _flt_header->set_auto_attr_update(_auto_attr_update);

  traverse(_data, _flt_header, FltGeometry::BT_none);

  // Finally, write the resulting file out.
  FltError result = _flt_header->write_flt(get_output());
  if (result != FE_ok) {
    nout << "Cannot write " << get_output_filename() << "\n";
    exit(1);
  }
}

/**
 * Dispatch function for the -attr parameter.
 */
bool EggToFlt::
dispatch_attr(const std::string &opt, const std::string &arg, void *var) {
  FltHeader::AttrUpdate *ip = (FltHeader::AttrUpdate *)var;

  if (cmp_nocase(arg, "none") == 0) {
    *ip = FltHeader::AU_none;

  } else if (cmp_nocase(arg, "new") == 0) {
    *ip = FltHeader::AU_if_missing;

  } else if (cmp_nocase(arg, "all") == 0) {
    *ip = FltHeader::AU_always;

  } else {
    nout << "-" << opt
         << " requires either \"none\", \"new\", or \"all\".\n";
    return false;
  }

  return true;
}

/**
 *
 */
void EggToFlt::
traverse(EggNode *egg_node, FltBead *flt_node,
         FltGeometry::BillboardType billboard) {
  if (egg_node->is_of_type(EggPolygon::get_class_type()) ||
      egg_node->is_of_type(EggPoint::get_class_type())) {
    // It's a polygon or point light.
    EggPrimitive *egg_primitive = DCAST(EggPrimitive, egg_node);
    convert_primitive(egg_primitive, flt_node, billboard);

  } else if (egg_node->is_of_type(EggExternalReference::get_class_type())) {
    // Convert external references.

  } else if (egg_node->is_of_type(EggGroup::get_class_type())) {
    // An EggGroup creates a fltBead, and recurses.
    EggGroup *egg_group = DCAST(EggGroup, egg_node);

    if (egg_group->get_group_type() == EggGroup::GT_joint) {
      // Ignore joints and their children.
      return;
    }

    convert_group(egg_group, flt_node, billboard);

  } else if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    // Some kind of grouping node other than an EggGroup.  Just recurse.
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);
    EggGroupNode::iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      traverse(*ci, flt_node, billboard);
    }
  }
}

/**
 * Converts an egg polygon or series of light points to the corresponding Flt
 * geometry, and adds it to the indicated flt_node.
 */
void EggToFlt::
convert_primitive(EggPrimitive *egg_primitive, FltBead *flt_node,
                  FltGeometry::BillboardType billboard) {
  FltFace *flt_face = new FltFace(_flt_header);
  flt_node->add_child(flt_face);

  flt_face->_billboard_type = billboard;

  if (egg_primitive->has_color()) {
    flt_face->set_color(egg_primitive->get_color());
  }

  if (egg_primitive->is_of_type(EggPoint::get_class_type())) {
    // A series of points, instead of a polygon.
    flt_face->_draw_type = FltFace::DT_omni_light;

  } else if (egg_primitive->get_bface_flag()) {
    // A polygon whose backface is visible.
    flt_face->_draw_type = FltFace::DT_solid_no_cull;

  } else {
    // A normal polygon.
    flt_face->_draw_type = FltFace::DT_solid_cull_backface;
  }

  if (egg_primitive->has_texture()) {
    EggTexture *egg_texture = egg_primitive->get_texture();
    FltTexture *flt_texture = get_flt_texture(egg_texture);
    flt_face->set_texture(flt_texture);
  }

  // Create a vertex list representing the vertices in the primitive, and add
  // it as a child of the face bead.  This is how Flt files associate vertices
  // with faces.
  FltVertexList *flt_vertices = new FltVertexList(_flt_header);
  flt_face->add_child(flt_vertices);

  EggPrimitive::iterator vi;
  bool all_verts_have_color = true;
  bool all_verts_have_normal = true;
  for (vi = egg_primitive->begin(); vi != egg_primitive->end(); ++vi) {
    EggVertex *egg_vertex = (*vi);
    FltVertex *flt_vertex = get_flt_vertex(egg_vertex, egg_primitive);
    flt_vertices->add_vertex(flt_vertex);

    if (!egg_vertex->has_color()) {
      all_verts_have_color = false;
    }
    if (!egg_vertex->has_normal()) {
      all_verts_have_normal = false;
    }
  }
  if (all_verts_have_color) {
    // If all the vertices of the face have a color specification, then we
    // specify per-vertex color on the face.
    if (all_verts_have_normal) {
      // And similarly with the normals.
      flt_face->_light_mode = FltFace::LM_vertex_with_normal;
    } else {
      flt_face->_light_mode = FltFace::LM_vertex_no_normal;
    }
  } else {
    if (all_verts_have_normal) {
      flt_face->_light_mode = FltFace::LM_face_with_normal;
    } else {
      flt_face->_light_mode = FltFace::LM_face_no_normal;
    }
  }
}

/**
 * Converts an egg group to the corresponding flt group, and adds it to the
 * indicated parent node.  Also recurses on the children of the egg group.
 */
void EggToFlt::
convert_group(EggGroup *egg_group, FltBead *flt_node,
              FltGeometry::BillboardType billboard) {
  std::ostringstream egg_syntax;

  FltGroup *flt_group = new FltGroup(_flt_header);
  flt_node->add_child(flt_group);

  flt_group->set_id(egg_group->get_name());

  switch (egg_group->get_billboard_type()) {
    // MultiGen represents billboarding at the polygon level, so we have to
    // remember this flag for later.
  case EggGroup::BT_axis:
    billboard = FltGeometry::BT_axial;
    break;

  case EggGroup::BT_point_world_relative:
    billboard = FltGeometry::BT_point;
    break;

  case EggGroup::BT_point_camera_relative:
    // Not sure if this is the right flag for MultiGen.
    billboard = FltGeometry::BT_fixed;
    break;

  default:
    break;
  }

  if (egg_group->has_transform()) {
    apply_transform(egg_group, flt_group);
  }

  if (egg_group->get_switch_flag()) {
    if (egg_group->get_switch_fps() != 0.0) {
      // A sequence animation.
      flt_group->_flags |= FltGroup::F_forward_animation;
      egg_syntax
        << "  <Scalar> fps { " << egg_group->get_switch_fps() << " }\n";
    } else {
      // Just a switch node.
      egg_group->write_switch_flags(egg_syntax, 2);
    }
  }

  // Pick up any additional egg attributes that MultiGen doesn't support;
  // these will get written to the comment field where flt2egg will find it.
  egg_group->write_collide_flags(egg_syntax, 2);
  egg_group->write_model_flags(egg_syntax, 2);
  egg_group->write_object_types(egg_syntax, 2);
  egg_group->write_decal_flags(egg_syntax, 2);
  egg_group->write_tags(egg_syntax, 2);
  egg_group->write_render_mode(egg_syntax, 2);

  apply_egg_syntax(egg_syntax.str(), flt_group);

  EggGroup::iterator ci;
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    traverse(*ci, flt_group, billboard);
  }
}

/**
 * Applies the indicated egg transform to the indicated flt bead.
 */
void EggToFlt::
apply_transform(EggTransform *egg_transform, FltBead *flt_node) {
  flt_node->clear_transform();

  bool components_ok = true;
  int num_components = egg_transform->get_num_components();
  for (int i = num_components - 1; i >= 0 && components_ok; i--) {
    switch (egg_transform->get_component_type(i)) {
    case EggTransform::CT_translate2d:
      {
        FltTransformTranslate *translate =
          new FltTransformTranslate(_flt_header);
        LVector2d v2 = egg_transform->get_component_vec2(i);
        translate->set(LPoint3d::zero(), LVector3d(v2[0], v2[1], 0.0));
        flt_node->add_transform_step(translate);
      }
      break;

    case EggTransform::CT_translate3d:
      {
        FltTransformTranslate *translate =
          new FltTransformTranslate(_flt_header);
        translate->set(LPoint3d::zero(), egg_transform->get_component_vec3(i));
        flt_node->add_transform_step(translate);
      }
      break;

    case EggTransform::CT_rotate2d:
      {
        FltTransformRotateAboutEdge *rotate =
          new FltTransformRotateAboutEdge(_flt_header);
        rotate->set(LPoint3d::zero(), LPoint3d(0.0, 0.0, 1.0),
                    egg_transform->get_component_number(i));
        flt_node->add_transform_step(rotate);
      }
      break;

    case EggTransform::CT_rotx:
      {
        FltTransformRotateAboutEdge *rotate =
          new FltTransformRotateAboutEdge(_flt_header);
        rotate->set(LPoint3d::zero(), LPoint3d(1.0, 0.0, 0.0),
                    egg_transform->get_component_number(i));
        flt_node->add_transform_step(rotate);
      }
      break;

    case EggTransform::CT_roty:
      {
        FltTransformRotateAboutEdge *rotate =
          new FltTransformRotateAboutEdge(_flt_header);
        rotate->set(LPoint3d::zero(), LPoint3d(0.0, 1.0, 0.0),
                    egg_transform->get_component_number(i));
        flt_node->add_transform_step(rotate);
      }
      break;

    case EggTransform::CT_rotz:
      {
        FltTransformRotateAboutEdge *rotate =
          new FltTransformRotateAboutEdge(_flt_header);
        rotate->set(LPoint3d::zero(), LPoint3d(0.0, 0.0, 1.0),
                    egg_transform->get_component_number(i));
        flt_node->add_transform_step(rotate);
      }
      break;

    case EggTransform::CT_rotate3d:
      {
        FltTransformRotateAboutEdge *rotate =
          new FltTransformRotateAboutEdge(_flt_header);
        rotate->set(LPoint3d::zero(), egg_transform->get_component_vec3(i),
                    egg_transform->get_component_number(i));
        flt_node->add_transform_step(rotate);
      }
      break;

    case EggTransform::CT_scale2d:
      {
        FltTransformScale *scale = new FltTransformScale(_flt_header);
        LVector2d v2 = egg_transform->get_component_vec2(i);
        scale->set(LPoint3d::zero(), LVector3(v2[0], v2[1], 1.0f));
        flt_node->add_transform_step(scale);
      }
      break;

    case EggTransform::CT_scale3d:
      {
        FltTransformScale *scale = new FltTransformScale(_flt_header);
        scale->set(LPoint3d::zero(), LCAST(PN_stdfloat, egg_transform->get_component_vec3(i)));
        flt_node->add_transform_step(scale);
      }
      break;

    case EggTransform::CT_uniform_scale:
      {
        FltTransformScale *scale = new FltTransformScale(_flt_header);
        PN_stdfloat factor = (PN_stdfloat)egg_transform->get_component_number(i);
        scale->set(LPoint3d::zero(), LVecBase3(factor, factor, factor));
        flt_node->add_transform_step(scale);
      }
      break;

    case EggTransform::CT_matrix3:
      {
        FltTransformGeneralMatrix *matrix =
          new FltTransformGeneralMatrix(_flt_header);
        const LMatrix3d &m = egg_transform->get_component_mat3(i);
        LMatrix4d mat4(m(0, 0), m(0, 1), 0.0, m(0, 2),
                       m(1, 0), m(1, 1), 0.0, m(1, 2),
                       0.0, 0.0, 1.0, 0.0,
                       m(2, 0), m(2, 1), 0.0, m(2, 2));
        matrix->set_matrix(mat4);
        flt_node->add_transform_step(matrix);
      }
      break;

    case EggTransform::CT_matrix4:
      {
        FltTransformGeneralMatrix *matrix =
          new FltTransformGeneralMatrix(_flt_header);
        matrix->set_matrix(egg_transform->get_component_mat4(i));
        flt_node->add_transform_step(matrix);
      }
      break;

    default:
      // Don't know how to convert this component.
      components_ok = false;
    }
  }

  if (components_ok) {
    // Verify that the transform was computed correctly.
    if (!flt_node->get_transform().almost_equal(egg_transform->get_transform3d())) {
      nout << "Incorrect transform!  Expected:\n";
      egg_transform->get_transform3d().write(nout, 2);
      nout << "Computed:\n";
      flt_node->get_transform().write(nout, 2);
      nout << "\n";
      components_ok = false;
    }
  }

  if (!components_ok) {
    // Just store the overall transform.
    flt_node->set_transform(egg_transform->get_transform3d());
  }
}

/**
 * Adds the indicated sequence of egg syntax lines (presumably representing
 * egg features not directly supported by MultiGen) to the flt record as a
 * comment, so that flt2egg will reapply it to the egg groups.
 */
void EggToFlt::
apply_egg_syntax(const std::string &egg_syntax, FltRecord *flt_record) {
  if (!egg_syntax.empty()) {
    std::ostringstream out;
    out << "<egg> {\n"
        << egg_syntax
        << "}";
    flt_record->set_comment(out.str());
  }
}

/**
 * Returns a FltVertex corresponding to the indicated EggVertex.  If the
 * vertex has not been seen before (in this particular vertex frame), creates
 * a new one.
 */
FltVertex *EggToFlt::
get_flt_vertex(EggVertex *egg_vertex, EggNode *context) {
  const LMatrix4d *frame = context->get_vertex_to_node_ptr();
  VertexMap &vertex_map = _vertex_map_per_frame[frame];

  VertexMap::iterator vi = vertex_map.find(egg_vertex);
  if (vi != vertex_map.end()) {
    return (*vi).second;
  }
  FltVertex *flt_vertex = new FltVertex(_flt_header);
  flt_vertex->_pos = egg_vertex->get_pos3();

  if (egg_vertex->has_color()) {
    flt_vertex->set_color(egg_vertex->get_color());
  }
  if (egg_vertex->has_normal()) {
    flt_vertex->_normal = LCAST(PN_stdfloat, egg_vertex->get_normal());
    flt_vertex->_has_normal = true;
  }
  if (egg_vertex->has_uv()) {
    flt_vertex->_uv = LCAST(PN_stdfloat, egg_vertex->get_uv());
    flt_vertex->_has_uv = true;
  }

  if (frame != nullptr) {
    flt_vertex->_pos = flt_vertex->_pos * (*frame);
    flt_vertex->_normal = flt_vertex->_normal * LCAST(PN_stdfloat, (*frame));
  }

  _flt_header->add_vertex(flt_vertex);
  vertex_map[egg_vertex] = flt_vertex;

  return flt_vertex;
}

/**
 * Returns a FltTexture corresponding to the indicated EggTexture.  If the
 * texture has not been seen before, creates a new one.
 */
FltTexture *EggToFlt::
get_flt_texture(EggTexture *egg_texture) {
  // We have to maintain this map based on the filename, not the egg pointer,
  // because there may be multiple EggTextures with the same filename, and we
  // have to collapse them together.
  Filename filename = egg_texture->get_filename();
  TextureMap::iterator vi = _texture_map.find(filename);
  if (vi != _texture_map.end()) {
    return (*vi).second;
  }
  FltTexture *flt_texture = new FltTexture(_flt_header);
  flt_texture->set_texture_filename(filename);

  switch (egg_texture->get_minfilter()) {
  case EggTexture::FT_nearest:
    flt_texture->_min_filter = FltTexture::MN_point;
    break;

  case EggTexture::FT_linear:
    flt_texture->_min_filter = FltTexture::MN_bilinear;
    break;

  case EggTexture::FT_nearest_mipmap_nearest:
    flt_texture->_min_filter = FltTexture::MN_mipmap_point;
    break;

  case EggTexture::FT_nearest_mipmap_linear:
    flt_texture->_min_filter = FltTexture::MN_mipmap_linear;
    break;

  case EggTexture::FT_linear_mipmap_nearest:
    flt_texture->_min_filter = FltTexture::MN_mipmap_bilinear;
    break;

  case EggTexture::FT_linear_mipmap_linear:
    flt_texture->_min_filter = FltTexture::MN_mipmap_trilinear;
    break;

  default:
    break;
  }

  switch (egg_texture->get_magfilter()) {
  case EggTexture::FT_nearest:
    flt_texture->_mag_filter = FltTexture::MG_point;
    break;

  case EggTexture::FT_linear:
    flt_texture->_mag_filter = FltTexture::MG_bilinear;
    break;

  default:
    break;
  }

  switch (egg_texture->get_wrap_mode()) {
  case EggTexture::WM_repeat:
    flt_texture->_repeat = FltTexture::RT_repeat;
    break;

  case EggTexture::WM_clamp:
    flt_texture->_repeat = FltTexture::RT_clamp;
    break;

  default:
    break;
  }

  switch (egg_texture->get_wrap_u()) {
  case EggTexture::WM_repeat:
    flt_texture->_repeat_u = FltTexture::RT_repeat;
    break;

  case EggTexture::WM_clamp:
    flt_texture->_repeat_u = FltTexture::RT_clamp;
    break;

  default:
    break;
  }

  switch (egg_texture->get_wrap_v()) {
  case EggTexture::WM_repeat:
    flt_texture->_repeat_v = FltTexture::RT_repeat;
    break;

  case EggTexture::WM_clamp:
    flt_texture->_repeat_v = FltTexture::RT_clamp;
    break;

  default:
    break;
  }

  switch (egg_texture->get_env_type()) {
  case EggTexture::ET_modulate:
    flt_texture->_env_type = FltTexture::ET_modulate;
    break;

  case EggTexture::ET_decal:
    flt_texture->_env_type = FltTexture::ET_decal;
    break;

  default:
    break;
  }

  switch (egg_texture->get_format()) {
  case EggTexture::F_luminance_alpha:
  case EggTexture::F_luminance_alphamask:
    flt_texture->_internal_format = FltTexture::IF_ia_8;
    break;

  case EggTexture::F_rgb5:
  case EggTexture::F_rgb332:
    flt_texture->_internal_format = FltTexture::IF_rgb_5;
    break;

  case EggTexture::F_rgba4:
  case EggTexture::F_rgba5:
    flt_texture->_internal_format = FltTexture::IF_rgba_4;
    break;

  case EggTexture::F_rgba8:
  case EggTexture::F_rgba:
  case EggTexture::F_rgbm:
  case EggTexture::F_rgb:
  case EggTexture::F_rgb8:
    flt_texture->_internal_format = FltTexture::IF_rgba_8;
    break;

  case EggTexture::F_rgba12:
    flt_texture->_internal_format = FltTexture::IF_rgba_12;
    break;

  case EggTexture::F_alpha:
    flt_texture->_internal_format = FltTexture::IF_i_16;
    flt_texture->_intensity_is_alpha = true;
    break;

  case EggTexture::F_luminance:
    flt_texture->_internal_format = FltTexture::IF_i_16;
    break;

  case EggTexture::F_rgb12:
    flt_texture->_internal_format = FltTexture::IF_rgb_12;
    break;

  default:
    break;
  }

  _flt_header->add_texture(flt_texture);
  _texture_map[filename] = flt_texture;

  return flt_texture;
}



int main(int argc, char *argv[]) {
  EggToFlt prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
