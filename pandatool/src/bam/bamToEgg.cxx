// Filename: bamToEgg.cxx
// Created by:  drose (25Jun01)
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

#include "bamToEgg.h"

#include "pandaNode.h"
#include "workingNodePath.h"
#include "nodePath.h"
#include "billboardEffect.h"
#include "renderEffects.h"
#include "transformState.h"
#include "colorScaleAttrib.h"
#include "colorAttrib.h"
#include "textureAttrib.h"
#include "cullFaceAttrib.h"
#include "lodNode.h"
#include "geomNode.h"
#include "geom.h"
#include "geomTri.h"
#include "string_utils.h"
#include "bamFile.h"
#include "eggGroup.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPrimitive.h"
#include "eggPolygon.h"
#include "eggTexture.h"
#include "eggMaterial.h"
#include "somethingToEggConverter.h"
#include "dcast.h"


////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BamToEgg::
BamToEgg() :
  SomethingToEgg("bam", ".bam")
{
  add_path_replace_options();
  add_path_store_options();

  set_program_description
    ("This program converts native Panda bam files to egg.  The conversion "
     "is somewhat incomplete; running egg2bam followed by bam2egg should not "
     "be expected to yield the same egg file you started with.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  By default, this is taken from the Config.prc file, which "
     "is currently " + format_string(get_default_coordinate_system()) + ".");

  _coordinate_system = get_default_coordinate_system();
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BamToEgg::
run() {
  BamFile bam_file;

  if (!bam_file.open_read(_input_filename)) {
    nout << "Unable to read " << _input_filename << "\n";
    exit(1);
  }

  nout << _input_filename << " : Bam version "
       << bam_file.get_file_major_ver() << "." 
       << bam_file.get_file_minor_ver() << "\n";

  typedef pvector<TypedWritable *> Objects;
  Objects objects;
  TypedWritable *object = bam_file.read_object();
  while (object != (TypedWritable *)NULL || !bam_file.is_eof()) {
    if (object != (TypedWritable *)NULL) {
      objects.push_back(object);
    }
    object = bam_file.read_object();
  }
  bam_file.resolve();
  bam_file.close();

  _data->set_coordinate_system(_coordinate_system);
  _vpool = new EggVertexPool("vpool");
  _data->add_child(_vpool);

  if (objects.size() == 1 && 
      objects[0]->is_of_type(PandaNode::get_class_type())) {
    PandaNode *node = DCAST(PandaNode, objects[0]);
    NodePath root(node);
    convert_node(WorkingNodePath(root), _data, false);

  } else {
    nout << "File does not contain a scene graph.\n";
    exit(1);
  }

  // Remove the vertex pool if it has no vertices.
  if (_vpool->empty()) {
    _data->remove_child(_vpool);
  }

  write_egg_file();
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::convert_node
//       Access: Private
//  Description: Converts the indicated node to the corresponding Egg
//               constructs, by first determining what kind of node it
//               is.
////////////////////////////////////////////////////////////////////
void BamToEgg::
convert_node(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
             bool has_decal) {
  PandaNode *node = node_path.node();
  if (node->is_geom_node()) {
    convert_geom_node(DCAST(GeomNode, node), node_path, egg_parent, has_decal);

  } else if (node->is_of_type(LODNode::get_class_type())) {
    convert_lod_node(DCAST(LODNode, node), node_path, egg_parent, has_decal);

  } else {
    // Just a generic node.
    EggGroup *egg_group = new EggGroup(node->get_name());
    egg_parent->add_child(egg_group);
    apply_node_properties(egg_group, node);
    
    recurse_nodes(node_path, egg_group, has_decal);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::convert_lod_node
//       Access: Private
//  Description: Converts the indicated LODNode to the corresponding
//               Egg constructs.
////////////////////////////////////////////////////////////////////
void BamToEgg::
convert_lod_node(LODNode *node, const WorkingNodePath &node_path,
                 EggGroupNode *egg_parent, bool has_decal) {
  // An LOD node gets converted to an ordinary EggGroup, but we apply
  // the appropriate switch conditions to each of our children.
  EggGroup *egg_group = new EggGroup(node->get_name());
  egg_parent->add_child(egg_group);
  apply_node_properties(egg_group, node);

  int num_children = node->get_num_children();
  int num_switches = node->get_num_switches();

  num_children = min(num_children, num_switches);

  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);

    // Convert just this one node to an EggGroup.
    PT(EggGroup) next_group = new EggGroup;
    convert_node(WorkingNodePath(node_path, child), next_group, has_decal);

    if (next_group->size() == 1) {
      // If we have exactly one child, and that child is an EggGroup,
      // collapse.
      EggNode *child_node = *next_group->begin();
      if (child_node->is_of_type(EggGroup::get_class_type())) {
        PT(EggGroup) child = DCAST(EggGroup, child_node);
        next_group->remove_child(child.p());
        next_group = child;
      }
    }

    // Now set up the switching properties appropriately.
    float in = node->get_in(i);
    float out = node->get_out(i);
    LPoint3f center = node->get_center();
    EggSwitchConditionDistance dist(in, out, LCAST(double, center));
    next_group->set_lod(dist);
    egg_group->add_child(next_group.p());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::convert_geom_node
//       Access: Private
//  Description: Converts a GeomNode to the corresponding egg
//               structures.
////////////////////////////////////////////////////////////////////
void BamToEgg::
convert_geom_node(GeomNode *node, const WorkingNodePath &node_path, 
                  EggGroupNode *egg_parent, bool has_decal) {
  PT(EggGroup) egg_group = new EggGroup(node->get_name());
  bool fancy_attributes = apply_node_properties(egg_group, node);

  if (node->get_effects()->has_decal()) {
    has_decal = true;
  }

  if (has_decal) {
    egg_group->set_decal_flag(true);
  }

  if (fancy_attributes || has_decal) {
    // If we have any fancy attributes on the node, or if we're making
    // decal geometry, we have to make a special node to hold the
    // geometry (normally it would just appear within its parent).
    egg_parent->add_child(egg_group.p());
    egg_parent = egg_group;
  }

  NodePath np = node_path.get_node_path();
  CPT(RenderState) net_state = np.get_net_state();
  CPT(TransformState) net_transform = np.get_net_transform();
  LMatrix4f net_mat = net_transform->get_mat();
  LMatrix4f inv = LCAST(float, egg_parent->get_vertex_frame_inv());
  net_mat = net_mat * inv;

  // Now get out all the various kinds of geometry.
  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; i++) {
    CPT(RenderState) geom_state = net_state->compose(node->get_geom_state(i));

    const Geom *geom = node->get_geom(i);
    // Explode the Geom before we try to deal with it.  That way, we
    // don't have to know about tristrips or whatnot.
    PT(Geom) exploded = geom->explode();

    // Now determine what kind of Geom we've got.  Chances are good
    // it's triangles.
    if (exploded->is_of_type(GeomTri::get_class_type())) {
      convert_geom_tri(DCAST(GeomTri, exploded), geom_state, net_mat,
                       egg_parent);
    }
  }
  
  recurse_nodes(node_path, egg_parent, has_decal);
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::convert_geom
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void BamToEgg::
convert_geom_tri(GeomTri *geom, const RenderState *net_state, 
                 const LMatrix4f &net_mat, EggGroupNode *egg_parent) {
  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomBindType vb = geom->get_binding(G_COORD);
  GeomBindType nb = geom->get_binding(G_NORMAL);
  GeomBindType tb = geom->get_binding(G_TEXCOORD);
  GeomBindType cb = geom->get_binding(G_COLOR);

  // Check for a color scale.
  LVecBase4f color_scale(1.0f, 1.0f, 1.0f, 1.0f);
  const RenderAttrib *color_scale_attrib = net_state->get_attrib(ColorScaleAttrib::get_class_type());
  if (color_scale_attrib != (const RenderAttrib *)NULL) {
    const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, color_scale_attrib);
    color_scale = csa->get_scale();
  }

  // Check for a color override.
  bool has_color_override = false;
  bool has_color_off = false;
  Colorf color_override;
  const RenderAttrib *color_attrib = net_state->get_attrib(ColorAttrib::get_class_type());
  if (color_attrib != (const RenderAttrib *)NULL) {
    const ColorAttrib *ca = DCAST(ColorAttrib, color_attrib);
    if (ca->get_color_type() == ColorAttrib::T_flat) {
      has_color_override = true;
      color_override = ca->get_color();
      color_override.set(color_override[0] * color_scale[0],
                         color_override[1] * color_scale[1],
                         color_override[2] * color_scale[2],
                         color_override[3] * color_scale[3]);

    } else if (ca->get_color_type() == ColorAttrib::T_off) {
      has_color_off = true;
    }
  }

  // Check for a texture.
  EggTexture *egg_tex = (EggTexture *)NULL;
  const RenderAttrib *tex_attrib = net_state->get_attrib(TextureAttrib::get_class_type());
  if (tex_attrib != (const RenderAttrib *)NULL) {
    const TextureAttrib *ta = DCAST(TextureAttrib, tex_attrib);
    egg_tex = get_egg_texture(ta->get_texture());
  }

  // Check the backface flag.
  bool bface = false;
  const RenderAttrib *cf_attrib = net_state->get_attrib(CullFaceAttrib::get_class_type());
  if (cf_attrib != (const RenderAttrib *)NULL) {
    const CullFaceAttrib *cfa = DCAST(CullFaceAttrib, cf_attrib);
    if (cfa->get_effective_mode() == CullFaceAttrib::M_cull_none) {
      bface = true;
    }
  }

  Normalf normal;
  Colorf color;

  // Get overall properties.
  if (nb == G_OVERALL) {
    normal = geom->get_next_normal(ni);
  }
  if (cb == G_OVERALL) {
    color = geom->get_next_color(ci);
  }

  for (int i = 0; i < nprims; i++) {
    // Get per-prim properties.
    if (nb == G_PER_PRIM) {
      normal = geom->get_next_normal(ni);
    }
    if (cb == G_PER_PRIM) {
      color = geom->get_next_color(ci);
    }

    EggPolygon *egg_poly = new EggPolygon;
    egg_parent->add_child(egg_poly);
    if (egg_tex != (EggTexture *)NULL) {
      egg_poly->set_texture(egg_tex);
    }

    if (bface) {
      egg_poly->set_bface_flag(true);
    }

    for (int j = 0; j < 3; j++) {
      EggVertex egg_vert;

      // Get per-vertex properties.
      if (vb == G_PER_VERTEX) {
        Vertexf vertex = geom->get_next_vertex(vi);
        egg_vert.set_pos(LCAST(double, vertex * net_mat));
      }
      if (nb == G_PER_VERTEX) {
        normal = geom->get_next_normal(ni) * net_mat;
      }
      if (tb == G_PER_VERTEX) {
        TexCoordf uv = geom->get_next_texcoord(ti);
        egg_vert.set_uv(LCAST(double, uv));
      }
      if (cb == G_PER_VERTEX) {
        color = geom->get_next_color(ci);
      }

      if (nb != G_OFF) {
        egg_vert.set_normal(LCAST(double, normal * net_mat));
      }

      if (has_color_override) {
        egg_vert.set_color(color_override);

      } else if (!has_color_off && cb != G_OFF) {
        egg_vert.set_color(Colorf(color[0] * color_scale[0],
                                  color[1] * color_scale[1],
                                  color[2] * color_scale[2],
                                  color[3] * color_scale[3]));
      }

      EggVertex *new_egg_vert = _vpool->create_unique_vertex(egg_vert);
      egg_poly->add_vertex(new_egg_vert);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::recurse_nodes
//       Access: Private
//  Description: Converts all the children of the indicated node.
////////////////////////////////////////////////////////////////////
void BamToEgg::
recurse_nodes(const WorkingNodePath &node_path, EggGroupNode *egg_parent,
              bool has_decal) {
  PandaNode *node = node_path.node();
  int num_children = node->get_num_children();
  
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);
    convert_node(WorkingNodePath(node_path, child), egg_parent, has_decal);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::apply_node_properties
//       Access: Public
//  Description: Applies any special properties that might be stored
//               on the node, like billboarding.  Returns true if any
//               were applied, false otherwise.
////////////////////////////////////////////////////////////////////
bool BamToEgg::
apply_node_properties(EggGroup *egg_group, PandaNode *node) {
  bool any_applied = false;

  if (node->get_draw_mask().is_zero()) {
    // This node is hidden.  We'll go ahead and convert it, but we'll
    // put in the "backstage" flag to mean it's not real geometry.
    egg_group->add_object_type("backstage");
  }

  const RenderEffects *effects = node->get_effects();
  const RenderEffect *effect = effects->get_effect(BillboardEffect::get_class_type());
  if (effect != (RenderEffect *)NULL) {
    const BillboardEffect *bbe = DCAST(BillboardEffect, effect);
    if (bbe->get_axial_rotate()) {
      egg_group->set_billboard_type(EggGroup::BT_axis);
      any_applied = true;

    } else if (bbe->get_eye_relative()) {
      egg_group->set_billboard_type(EggGroup::BT_point_camera_relative);
      any_applied = true;

    } else {
      egg_group->set_billboard_type(EggGroup::BT_point_world_relative);
      any_applied = true;
    }
  }

  const TransformState *transform = node->get_transform();
  if (!transform->is_identity()) {
    if (transform->has_components()) {
      // If the transform can be represented componentwise, we prefer
      // storing it that way in the egg file.
      const LVecBase3f &scale = transform->get_scale();
      const LQuaternionf &quat = transform->get_quat();
      const LVecBase3f &pos = transform->get_pos();
      if (!scale.almost_equal(LVecBase3f(1.0f, 1.0f, 1.0f))) {
        egg_group->add_scale(LCAST(double, scale));
      }
      if (!quat.is_identity()) {
        egg_group->add_rotate(LCAST(double, quat));
      }
      if (!pos.almost_equal(LVecBase3f::zero())) {
        egg_group->add_translate(LCAST(double, pos));
      }

    } else if (transform->has_mat()) {
      // Otherwise, we store the raw matrix.
      const LMatrix4f &mat = transform->get_mat();
      egg_group->set_transform(LCAST(double, mat));
    }
    any_applied = true;
  }

  return any_applied;
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::get_egg_texture
//       Access: Public
//  Description: Returns an EggTexture pointer that corresponds to the
//               indicated Texture.
////////////////////////////////////////////////////////////////////
EggTexture *BamToEgg::
get_egg_texture(Texture *tex) {
  if (tex != (Texture *)NULL) {
    if (tex->has_filename()) {
      Filename filename = _path_replace->convert_path(tex->get_filename());
      EggTexture temp(filename.get_basename_wo_extension(), filename);
      if (tex->has_alpha_filename()) {
        Filename alpha = _path_replace->convert_path(tex->get_alpha_filename());
        temp.set_alpha_filename(alpha);
      }

      switch (tex->get_minfilter()) {
      case Texture::FT_invalid:
        break;
      case Texture::FT_nearest:
        temp.set_minfilter(EggTexture::FT_nearest);
        break;
      case Texture::FT_linear:
        temp.set_minfilter(EggTexture::FT_linear);
        break;
      case Texture::FT_nearest_mipmap_nearest:
        temp.set_minfilter(EggTexture::FT_nearest_mipmap_nearest);
        break;
      case Texture::FT_linear_mipmap_nearest:
        temp.set_minfilter(EggTexture::FT_linear_mipmap_nearest);
        break;
      case Texture::FT_nearest_mipmap_linear:
        temp.set_minfilter(EggTexture::FT_nearest_mipmap_linear);
        break;
      case Texture::FT_linear_mipmap_linear:
        temp.set_minfilter(EggTexture::FT_linear_mipmap_linear);
        break;
      }

      switch (tex->get_magfilter()) {
      case Texture::FT_nearest:
        temp.set_magfilter(EggTexture::FT_nearest);
        break;
      case Texture::FT_linear:
        temp.set_magfilter(EggTexture::FT_linear);
        break;
      default:
        break;
      }

      switch (tex->get_wrap_u()) {
      case Texture::WM_clamp:
        temp.set_wrap_u(EggTexture::WM_clamp);
        break;
      case Texture::WM_repeat:
        temp.set_wrap_u(EggTexture::WM_repeat);
        break;

      default:
        // There are some new wrap options on Texture that aren't yet
        // supported in egg.
        break;
      }

      switch (tex->get_wrap_v()) {
      case Texture::WM_clamp:
        temp.set_wrap_v(EggTexture::WM_clamp);
        break;
      case Texture::WM_repeat:
        temp.set_wrap_v(EggTexture::WM_repeat);
        break;

      default:
        // There are some new wrap options on Texture that aren't yet
        // supported in egg.
        break;
      }

      switch (tex->get_format()) {
      case Texture::F_red:
        temp.set_format(EggTexture::F_red);
        break;
      case Texture::F_green:
        temp.set_format(EggTexture::F_green);
        break;
      case Texture::F_blue:
        temp.set_format(EggTexture::F_blue);
        break;
      case Texture::F_alpha:
        temp.set_format(EggTexture::F_alpha);
        break;
      case Texture::F_rgb:
        temp.set_format(EggTexture::F_rgb);
        break;
      case Texture::F_rgb5:
        temp.set_format(EggTexture::F_rgb5);
        break;
      case Texture::F_rgb8:
        temp.set_format(EggTexture::F_rgb8);
        break;
      case Texture::F_rgb12:
        temp.set_format(EggTexture::F_rgb12);
        break;
      case Texture::F_rgb332:
        temp.set_format(EggTexture::F_rgb332);
        break;
      case Texture::F_rgba:
        temp.set_format(EggTexture::F_rgba);
        break;
      case Texture::F_rgbm:
        temp.set_format(EggTexture::F_rgbm);
        break;
      case Texture::F_rgba4:
        temp.set_format(EggTexture::F_rgba4);
        break;
      case Texture::F_rgba5:
        temp.set_format(EggTexture::F_rgba5);
        break;
      case Texture::F_rgba8:
        temp.set_format(EggTexture::F_rgba8);
        break;
      case Texture::F_rgba12:
        temp.set_format(EggTexture::F_rgba12);
        break;
      case Texture::F_luminance:
        temp.set_format(EggTexture::F_luminance);
        break;
      case Texture::F_luminance_alpha:
        temp.set_format(EggTexture::F_luminance_alpha);
        break;
      case Texture::F_luminance_alphamask:
        temp.set_format(EggTexture::F_luminance_alphamask);
        break;
      default:
        break;
      }

      return _textures.create_unique_texture(temp, ~EggTexture::E_tref_name);
    }
  }

  return NULL;
}


int main(int argc, char *argv[]) {
  BamToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
