// Filename: bamToEgg.cxx
// Created by:  drose (25Jun01)
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

#include "bamToEgg.h"

#include "node.h"
#include "LODNode.h"
#include "geomNode.h"
#include "geom.h"
#include "geomTri.h"
#include "renderRelation.h"
#include "string_utils.h"
#include "bamFile.h"
#include "eggGroup.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPrimitive.h"
#include "eggPolygon.h"
#include "eggTexture.h"
#include "eggMaterial.h"
#include "arcChain.h"
#include "allTransitionsWrapper.h"
#include "wrt.h"
#include "transformTransition.h"
#include "texMatrixTransition.h"
#include "colorMatrixTransition.h"
#include "alphaTransformTransition.h"
#include "textureTransition.h"
#include "cullFaceTransition.h"
#include "cullFaceProperty.h"
#include "billboardTransition.h"
#include "decalTransition.h"
#include "somethingToEggConverter.h"


////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::GeomState::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
BamToEgg::GeomState::
GeomState() {
  _mat = LMatrix4f::ident_mat();
  _tex_mat = LMatrix4f::ident_mat();
  _color_mat = LMatrix4f::ident_mat();
  _alpha_scale = 1.0;
  _alpha_offset = 0.0;
  _tex = (Texture *)NULL;
  _bface = false;
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::GeomState::get_net_state
//       Access: Public
//  Description: Gets the accumulated state of the indicated node (and
//               its corresponding ArcChain) into the GeomState.
////////////////////////////////////////////////////////////////////
void BamToEgg::GeomState::
get_net_state(Node *node, ArcChain &chain, EggGroupNode *egg_parent) {
  AllTransitionsWrapper atw;

  wrt(node, chain.begin(), chain.end(),
      (Node *)NULL,
      atw, RenderRelation::get_class_type());

  // Check for transform space.
  const TransformTransition *tt;
  if (get_transition_into(tt, atw)) {
    _mat = tt->get_matrix();
    LMatrix4f inv = LCAST(float, egg_parent->get_vertex_frame_inv());
    _mat = _mat * inv;
  }

  // Check for texture matrix.
  const TexMatrixTransition *tmt;
  if (get_transition_into(tmt, atw)) {
    _tex_mat = tmt->get_matrix();
  }

  // Check for color matrix.
  const ColorMatrixTransition *cmt;
  if (get_transition_into(cmt, atw)) {
    _color_mat = cmt->get_matrix();
  }

  // Check for alpha scale/offset.
  const AlphaTransformTransition *att;
  if (get_transition_into(att, atw)) {
    _alpha_scale = att->get_scale();
    _alpha_offset = att->get_offset();
  }

  // Check for texture.
  const TextureTransition *txt;
  if (get_transition_into(txt, atw)) {
    if (txt->is_on()) {
      _tex = txt->get_texture();
    }
  }

  // Check for bface.
  const CullFaceTransition *cft;
  if (get_transition_into(cft, atw)) {
    if (cft->get_mode() == CullFaceProperty::M_cull_none) {
      _bface = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::GeomState::apply_vertex
//       Access: Public
//  Description: Applies the indicated vertex coordinate to the given
//               EggVertex, after modifying it according to the
//               current state.
////////////////////////////////////////////////////////////////////
void BamToEgg::GeomState::
apply_vertex(EggVertex &egg_vert, const Vertexf &vertex) {
  LPoint3f transformed = vertex * _mat;
  egg_vert.set_pos(LCAST(double, transformed));
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::GeomState::apply_normal
//       Access: Public
//  Description: Applies the indicated vertex normal to the given
//               EggVertex, after modifying it according to the
//               current state.
////////////////////////////////////////////////////////////////////
void BamToEgg::GeomState::
apply_normal(EggVertex &egg_vert, const Normalf &normal) {
  LPoint3f transformed = normal * _mat;
  egg_vert.set_normal(LCAST(double, transformed));
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::GeomState::apply_uv
//       Access: Public
//  Description: Applies the indicated vertex UV to the given
//               EggVertex, after modifying it according to the
//               current state.
////////////////////////////////////////////////////////////////////
void BamToEgg::GeomState::
apply_uv(EggVertex &egg_vert, const TexCoordf &uv) {
  LVecBase4f v4(uv[0], uv[1], 0.0, 1.0);
  v4 = v4 * _tex_mat;
  egg_vert.set_uv(LCAST(double, uv));
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::GeomState::apply_color
//       Access: Public
//  Description: Applies the indicated vertex color to the given
//               EggVertex, after modifying it according to the
//               current state.
////////////////////////////////////////////////////////////////////
void BamToEgg::GeomState::
apply_color(EggVertex &egg_vert, const Colorf &color) {
  LPoint3f temp(color[0], color[1], color[2]);
  temp = temp * _color_mat;
  float alpha = (color[3] * _alpha_scale) +
    _alpha_offset;
  
  Colorf transformed(temp[0], temp[1], temp[2], alpha);
  egg_vert.set_color(transformed);
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::GeomState::apply_prim
//       Access: Public
//  Description: Applies generic, non-vertex-specific properties to
//               the primitive.
////////////////////////////////////////////////////////////////////
void BamToEgg::GeomState::
apply_prim(EggPrimitive *egg_prim) {
  if (_bface) {
    egg_prim->set_bface_flag(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BamToEgg::
BamToEgg() :
  SomethingToEgg("Bam", ".bam")
{
  add_texture_path_options();
  add_rel_dir_options();
  add_search_path_options(true);

  set_program_description
    ("This program converts native Panda Bam files to egg.  The conversion "
     "is somewhat incomplete; running egg2bam followed by bam2egg should not "
     " be expected to yield the same egg file you started with.");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  By default, this is taken from the Configrc file, which "
     "is currently " + format_string(default_coordinate_system) + ".");

  _coordinate_system = default_coordinate_system;
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

  _data.set_coordinate_system(_coordinate_system);
  _vpool = new EggVertexPool("vpool");
  _data.add_child(_vpool);

  if (objects.size() == 1 && objects[0]->is_of_type(Node::get_class_type())) {
    Node *node = DCAST(Node, objects[0]);
    ArcChain chain(node);
    convert_node(node, chain, &_data);

  } else {
    nout << "File does not contain a scene graph.\n";
    exit(1);
  }

  // Remove the vertex pool if it has no vertices.
  if (_vpool->empty()) {
    _data.remove_child(_vpool);
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
convert_node(Node *node, ArcChain &chain, EggGroupNode *egg_parent) {
  if (node->is_of_type(LODNode::get_class_type())) {
    convert_lod_node(DCAST(LODNode, node), chain, egg_parent);

  } else if (node->is_of_type(GeomNode::get_class_type())) {
    convert_geom_node(DCAST(GeomNode, node), chain, egg_parent);

  } else {
    // Just a generic node.  See if it has a name, at least.
    string name;
    if (node->is_of_type(NamedNode::get_class_type())) {
      name = DCAST(NamedNode, node)->get_name();
    }
    
    EggGroup *egg_group = new EggGroup(name);
    egg_parent->add_child(egg_group);
    apply_arc_properties(egg_group, chain);
    
    recurse_nodes(node, chain, egg_group);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::convert_lod_node
//       Access: Private
//  Description: Converts the indicated LODNode to the corresponding
//               Egg constructs.
////////////////////////////////////////////////////////////////////
void BamToEgg::
convert_lod_node(LODNode *node, ArcChain &chain, EggGroupNode *egg_parent) {
  // An LOD node gets converted to an ordinary EggGroup, but we apply
  // the appropriate switch conditions to each of our children.
  string name = node->get_name();

  EggGroup *egg_group = new EggGroup(name);
  egg_parent->add_child(egg_group);
  apply_arc_properties(egg_group, chain);

  int num_children = node->get_num_children(RenderRelation::get_class_type());
  int num_switches = node->get_num_switches();

  num_children = min(num_children, num_switches);

  for (int i = 0; i < num_children; i++) {
    NodeRelation *arc = 
      node->get_child(RenderRelation::get_class_type(), i);
    ArcChain next_chain(chain);
    next_chain.push_back(arc);

    // Convert just this one node to an EggGroup.
    PT(EggGroup) next_group = new EggGroup;
    convert_node(arc->get_child(), next_chain, next_group);

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
    LPoint3f center = node->_lod._center;
    EggSwitchConditionDistance dist(in, out, LCAST(double, center));
    next_group->set_lod(dist);
    egg_group->add_child(next_group.p());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::convert_geom_node
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void BamToEgg::
convert_geom_node(GeomNode *node, ArcChain &chain, EggGroupNode *egg_parent) {
  bool parent_has_decal = false;
  if (egg_parent->is_of_type(EggGroup::get_class_type())) {
    EggGroup *pg = DCAST(EggGroup, egg_parent);
    parent_has_decal = pg->get_decal_flag();
  }

  PT(EggGroup) egg_group = new EggGroup(node->get_name());
  bool fancy_transitions = apply_arc_properties(egg_group, chain);
  if (fancy_transitions || parent_has_decal) {
    // If we have any fancy transitions on the arc, or if we're making
    // decal geometry, we have to make a special node to hold the
    // geometry (normally it would just appear within its parent).
    egg_parent->add_child(egg_group.p());
    egg_parent = egg_group;
  }
    

  // Get the state associated with this GeomNode.
  GeomState state;
  state.get_net_state(node, chain, egg_parent);

  // Now get out all the various kinds of geometry.
  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; i++) {
    dDrawable *drawable = node->get_geom(i);
    if (drawable->is_of_type(Geom::get_class_type())) {
      // Explode the Geom before we try to deal with it.  That way, we
      // don't have to know about tristrips or whatnot.
      Geom *geom = DCAST(Geom, drawable);
      PT(Geom) exploded = geom->explode();

      // Now determine what kind of Geom we've got.  Chances are good
      // it's triangles.
      if (exploded->is_of_type(GeomTri::get_class_type())) {
        convert_geom_tri(DCAST(GeomTri, exploded), state, egg_parent);
      }
    }
  }
  
  recurse_nodes(node, chain, egg_parent);
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::convert_geom
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void BamToEgg::
convert_geom_tri(GeomTri *geom, BamToEgg::GeomState &state, EggGroupNode *egg_parent) {
  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomBindType vb = geom->get_binding(G_COORD);
  GeomBindType nb = geom->get_binding(G_NORMAL);
  GeomBindType tb = geom->get_binding(G_TEXCOORD);
  GeomBindType cb = geom->get_binding(G_COLOR);

  Vertexf vertex;
  Normalf normal;
  TexCoordf uv;
  Colorf color;

  // Get overall properties.
  if (vb == G_OVERALL) {
    vertex = geom->get_next_vertex(vi);
  }
  if (nb == G_OVERALL) {
    normal = geom->get_next_normal(ni);
  }
  if (tb == G_OVERALL) {
    uv = geom->get_next_texcoord(ti);
  }
  if (cb == G_OVERALL) {
    color = geom->get_next_color(ci);
  }

  for (int i = 0; i < nprims; i++) {
    // Get per-prim properties.
    if (vb == G_PER_PRIM) {
      vertex = geom->get_next_vertex(vi);
    }
    if (nb == G_PER_PRIM) {
      normal = geom->get_next_normal(ni);
    }
    if (tb == G_PER_PRIM) {
      uv = geom->get_next_texcoord(ti);
    }
    if (cb == G_PER_PRIM) {
      color = geom->get_next_color(ci);
    }

    EggPolygon *egg_poly = new EggPolygon;
    egg_parent->add_child(egg_poly);
    apply_texture(egg_poly, state._tex);
    state.apply_prim(egg_poly);

    for (int j = 0; j < 3; j++) {
      // Get per-vertex properties.
      if (vb == G_PER_VERTEX) {
        vertex = geom->get_next_vertex(vi);
      }
      if (nb == G_PER_VERTEX) {
        normal = geom->get_next_normal(ni);
      }
      if (tb == G_PER_VERTEX) {
        uv = geom->get_next_texcoord(ti);
      }
      if (cb == G_PER_VERTEX) {
        color = geom->get_next_color(ci);
      }

      EggVertex egg_vert;
      state.apply_vertex(egg_vert, vertex);
      if (nb != G_OFF) {
        state.apply_normal(egg_vert, normal);
      }
      if (tb != G_OFF) {
        state.apply_uv(egg_vert, uv);
      }
      if (cb != G_OFF) {
        state.apply_color(egg_vert, color);
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
recurse_nodes(Node *node, ArcChain &chain, EggGroupNode *egg_parent) {
  int num_children = node->get_num_children(RenderRelation::get_class_type());

  for (int i = 0; i < num_children; i++) {
    NodeRelation *arc = 
      node->get_child(RenderRelation::get_class_type(), i);
    ArcChain next_chain(chain);
    next_chain.push_back(arc);
    convert_node(arc->get_child(), next_chain, egg_parent);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::apply_texture
//       Access: Public
//  Description: Applies the texture, if any, to the indicated
//               EggPrimitive.
////////////////////////////////////////////////////////////////////
void BamToEgg::
apply_texture(EggPrimitive *egg_prim, Texture *tex) {
  if (tex != (Texture *)NULL) {
    if (tex->has_name()) {
      Filename filename = SomethingToEggConverter::convert_path
        (tex->get_name(), get_texture_path(), _make_rel_dir, 
         _texture_path_convert);

      EggTexture temp("", filename);
      if (tex->has_alpha_name()) {
        Filename alpha = SomethingToEggConverter::convert_path
          (tex->get_alpha_name(), get_texture_path(), _make_rel_dir, 
           _texture_path_convert);
        temp.set_alpha_file(alpha);
      }

      switch (tex->get_minfilter()) {
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

      switch (tex->get_wrapu()) {
      case Texture::WM_clamp:
        temp.set_wrap_u(EggTexture::WM_clamp);
        break;
      case Texture::WM_repeat:
        temp.set_wrap_u(EggTexture::WM_repeat);
        break;
      }

      switch (tex->get_wrapv()) {
      case Texture::WM_clamp:
        temp.set_wrap_v(EggTexture::WM_clamp);
        break;
      case Texture::WM_repeat:
        temp.set_wrap_v(EggTexture::WM_repeat);
        break;
      }

      PixelBuffer *pbuf = tex->get_ram_image();
      if (pbuf != (PixelBuffer *)NULL) {
        switch (pbuf->get_format()) {
        case PixelBuffer::F_red:
          temp.set_format(EggTexture::F_red);
          break;
        case PixelBuffer::F_green:
          temp.set_format(EggTexture::F_green);
          break;
        case PixelBuffer::F_blue:
          temp.set_format(EggTexture::F_blue);
          break;
        case PixelBuffer::F_alpha:
          temp.set_format(EggTexture::F_alpha);
          break;
        case PixelBuffer::F_rgb:
          temp.set_format(EggTexture::F_rgb);
          break;
        case PixelBuffer::F_rgb5:
          temp.set_format(EggTexture::F_rgb5);
          break;
        case PixelBuffer::F_rgb8:
          temp.set_format(EggTexture::F_rgb8);
          break;
        case PixelBuffer::F_rgb12:
          temp.set_format(EggTexture::F_rgb12);
          break;
        case PixelBuffer::F_rgb332:
          temp.set_format(EggTexture::F_rgb332);
          break;
        case PixelBuffer::F_rgba:
          temp.set_format(EggTexture::F_rgba);
          break;
        case PixelBuffer::F_rgbm:
          temp.set_format(EggTexture::F_rgbm);
          break;
        case PixelBuffer::F_rgba4:
          temp.set_format(EggTexture::F_rgba4);
          break;
        case PixelBuffer::F_rgba5:
          temp.set_format(EggTexture::F_rgba5);
          break;
        case PixelBuffer::F_rgba8:
          temp.set_format(EggTexture::F_rgba8);
          break;
        case PixelBuffer::F_rgba12:
          temp.set_format(EggTexture::F_rgba12);
          break;
        case PixelBuffer::F_luminance:
          temp.set_format(EggTexture::F_luminance);
          break;
        case PixelBuffer::F_luminance_alpha:
          temp.set_format(EggTexture::F_luminance_alpha);
          break;
        case PixelBuffer::F_luminance_alphamask:
          temp.set_format(EggTexture::F_luminance_alphamask);
          break;
        default:
          break;
        }
      }

      EggTexture *egg_tex = 
        _textures.create_unique_texture(temp, ~EggTexture::E_tref_name);
      egg_prim->set_texture(egg_tex);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamToEgg::apply_arc_properties
//       Access: Public
//  Description: Applies any special properties that might be stored
//               on the arc, like billboarding.  Returns true if any
//               were applied, false otherwise.
////////////////////////////////////////////////////////////////////
bool BamToEgg::
apply_arc_properties(EggGroup *egg_group, ArcChain &chain) {
  if (!chain.has_arcs()) {
    return false;
  }
  NodeRelation *arc = chain.back();
  bool any_applied = false;

  BillboardTransition *bt;
  if (get_transition_into(bt, arc)) {
    if (bt->get_axial_rotate()) {
      egg_group->set_billboard_type(EggGroup::BT_axis);
      any_applied = true;

    } else if (bt->get_eye_relative()) {
      egg_group->set_billboard_type(EggGroup::BT_point_camera_relative);
      any_applied = true;

    } else {
      egg_group->set_billboard_type(EggGroup::BT_point_world_relative);
      any_applied = true;
    }
  }

  const TransformTransition *tt;
  if (get_transition_into(tt, arc)) {
    LMatrix4f mat = tt->get_matrix();
    egg_group->set_transform(LCAST(double, mat));
    any_applied = true;
  }

  const DecalTransition *dt;
  if (get_transition_into(dt, arc)) {
    if (dt->is_on()) {
      egg_group->set_decal_flag(true);
      any_applied = true;
    }
  }

  return true;
}


int main(int argc, char *argv[]) {
  BamToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
