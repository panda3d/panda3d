// Filename: EggLoader.cxx
// Created by:  drose (26Feb02)
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

#include "pandabase.h"

#include "eggLoader.h"
#include "egg_parametrics.h"
#include "config_egg2pg.h"
#include "nodePath.h"
#include "renderState.h"
#include "transformState.h"
#include "textureAttrib.h"
#include "textureApplyAttrib.h"
#include "texturePool.h"
#include "billboardEffect.h"
#include "cullFaceAttrib.h"
#include "cullBinAttrib.h"
#include "transparencyAttrib.h"
#include "decalEffect.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "materialAttrib.h"
#include "texMatrixAttrib.h"
#include "colorAttrib.h"
#include "materialPool.h"
#include "geomNode.h"
#include "sequenceNode.h"
#include "switchNode.h"
#include "portalNode.h"
#include "polylightNode.h"
#include "lodNode.h"
#include "modelNode.h"
#include "modelRoot.h"
#include "string_utils.h"
#include "eggPrimitive.h"
#include "eggPoint.h"
#include "eggLine.h"
#include "eggTextureCollection.h"
#include "eggNurbsCurve.h"
#include "eggNurbsSurface.h"
#include "eggGroupNode.h"
#include "eggGroup.h"
#include "eggPolygon.h"
#include "eggBin.h"
#include "eggTable.h"
#include "eggBinner.h"
#include "eggVertexPool.h"
#include "pt_EggTexture.h"
#include "characterMaker.h"
#include "character.h"
#include "animBundleMaker.h"
#include "animBundleNode.h"
#include "selectiveChildNode.h"
#include "collisionNode.h"
#include "collisionSphere.h"
#include "collisionTube.h"
#include "collisionPlane.h"
#include "collisionPolygon.h"
#include "parametricCurve.h"
#include "nurbsCurve.h"
#include "classicNurbsCurve.h"
#include "nurbsCurveInterface.h"
#include "nurbsCurveEvaluator.h"
#include "nurbsSurfaceEvaluator.h"
#include "ropeNode.h"
#include "sheetNode.h"
#include "look_at.h"

#include <ctype.h>
#include <algorithm>

// This class is used in make_node(EggBin *) to sort LOD instances in
// order by switching distance.
class LODInstance {
public:
  LODInstance(EggNode *egg_node);
  bool operator < (const LODInstance &other) const {
    return _d->_switch_in < other._d->_switch_in;
  }

  EggNode *_egg_node;
  const EggSwitchConditionDistance *_d;
};

LODInstance::
LODInstance(EggNode *egg_node) {
  nassertv(egg_node != NULL);
  _egg_node = egg_node;

  // We expect this egg node to be an EggGroup with an LOD
  // specification.  That's what the EggBinner collected together,
  // after all.
  EggGroup *egg_group = DCAST(EggGroup, egg_node);
  nassertv(egg_group->has_lod());
  const EggSwitchCondition &sw = egg_group->get_lod();

  // For now, this is the only kind of switch condition there is.
  _d = DCAST(EggSwitchConditionDistance, &sw);
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggLoader::
EggLoader() {
  // We need to enforce whatever coordinate system the user asked for.
  _data.set_coordinate_system(egg_coordinate_system);
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggLoader::
EggLoader(const EggData &data) :
  _data(data)
{
  _error = false;
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::build_graph
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggLoader::
build_graph() {
  _deferred_nodes.clear();

  // First, bin up the LOD nodes.
  EggBinner binner;
  binner.make_bins(&_data);

  // Then load up all of the textures.
  load_textures();

  // Now build up the scene graph.
  _root = new ModelRoot(_data.get_egg_filename().get_basename());
  make_node(&_data, _root);
  _builder.build();

  reparent_decals();

  apply_deferred_nodes(_root, DeferredNodeProperty());
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::reparent_decals
//       Access: Public
//  Description: For each node representing a decal base geometry
//               (i.e. a node corresponding to an EggGroup with the
//               decal flag set), move all of its nested geometry
//               directly below the GeomNode representing the group.
////////////////////////////////////////////////////////////////////
void EggLoader::
reparent_decals() {
  Decals::const_iterator di;
  for (di = _decals.begin(); di != _decals.end(); ++di) {
    PandaNode *node = (*di);
    nassertv(node != (PandaNode *)NULL);

    // The NodePath interface is best for this.
    NodePath parent(node);

    // First, search for the GeomNode.
    NodePath geom_parent;
    int num_children = parent.get_num_children();
    for (int i = 0; i < num_children; i++) {
      NodePath child = parent.get_child(i);

      if (child.node()->is_of_type(GeomNode::get_class_type())) {
        if (!geom_parent.is_empty()) {
          // Oops, too many GeomNodes.
          egg2pg_cat.error()
            << "Decal onto " << parent.node()->get_name()
            << " uses base geometry with multiple GeomNodes.\n";
          _error = true;
        }
        geom_parent = child;
      }
    }

    if (geom_parent.is_empty()) {
      // No children were GeomNodes.
      egg2pg_cat.error()
        << "Ignoring decal onto " << parent.node()->get_name()
        << "; no geometry within group.\n";
      _error = true;
    } else {
      // Now reparent all of the non-GeomNodes to this node.  We have
      // to be careful so we don't get lost as we self-modify this
      // list.
      int i = 0;
      while (i < num_children) {
        NodePath child = parent.get_child(i);

        if (child.node()->is_of_type(GeomNode::get_class_type())) {
          i++;
        } else {
          child.reparent_to(geom_parent);
          num_children--;
        }
      }

      // Finally, set the DecalEffect on the base geometry.
      geom_parent.node()->set_effect(DecalEffect::make());
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_nonindexed_primitive
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggLoader::
make_nonindexed_primitive(EggPrimitive *egg_prim, PandaNode *parent,
                          const LMatrix4d *transform) {
  BuilderBucket bucket;
  BakeInUVs bake_in_uvs;
  setup_bucket(bucket, bake_in_uvs, parent, egg_prim);
  if (bucket._hidden && egg_suppress_hidden) {
    // Eat this primitive.
    return;
  }

  LMatrix4d mat;

  if (transform != NULL) {
    mat = (*transform);
  } else {
    mat = egg_prim->get_vertex_to_node();
  }

  if (egg_prim->is_of_type(EggNurbsCurve::get_class_type())) {
    make_nurbs_curve(DCAST(EggNurbsCurve, egg_prim), parent, mat);

  } else if (egg_prim->is_of_type(EggNurbsSurface::get_class_type())) {
    make_nurbs_surface(DCAST(EggNurbsSurface, egg_prim), parent, mat);

  } else {
    // A normal primitive: polygon, line, or point.
    BuilderPrim bprim;
    bprim.set_type(BPT_poly);
    if (egg_prim->is_of_type(EggLine::get_class_type())) {
      bprim.set_type(BPT_linestrip);
    } else if (egg_prim->is_of_type(EggPoint::get_class_type())) {
      bprim.set_type(BPT_point);
    }
    
    if (egg_prim->has_normal()) {
      Normald norm = egg_prim->get_normal() * mat;
      norm.normalize();
      bprim.set_normal(LCAST(float, norm));
    }
    if (egg_prim->has_color() && !egg_false_color) {
      bprim.set_color(egg_prim->get_color());
    }
    
    bool has_vert_color = true;
    EggPrimitive::const_iterator vi;
    for (vi = egg_prim->begin(); vi != egg_prim->end(); ++vi) {
      EggVertex *egg_vert = *vi;
      if (egg_vert->get_num_dimensions() != 3) {
        egg2pg_cat.error()
          << "Vertex " << egg_vert->get_pool()->get_name() 
          << ":" << egg_vert->get_index() << " has dimension " 
          << egg_vert->get_num_dimensions() << "\n";
      } else {
        BuilderVertex bvert(LCAST(float, egg_vert->get_pos3() * mat));
        
        if (egg_vert->has_normal()) {
          Normald norm = egg_vert->get_normal() * mat;
          norm.normalize();
          bvert.set_normal(LCAST(float, norm));
        }
        if (egg_vert->has_color() && !egg_false_color) {
          bvert.set_color(egg_vert->get_color());
        } else {
          // If any vertex doesn't have a color, we can't use any of the
          // vertex colors.
          has_vert_color = false;
        }

        EggVertex::const_uv_iterator ui;
        for (ui = egg_vert->uv_begin(); ui != egg_vert->uv_end(); ++ui) {
          EggVertexUV *uv_obj = (*ui);
          TexCoordd uv = uv_obj->get_uv();
          CPT(TexCoordName) uv_name;
          if (uv_obj->has_name()) {
            uv_name = TexCoordName::make(uv_obj->get_name());
          } else {
            uv_name = TexCoordName::get_default();
          }

          BakeInUVs::const_iterator buv = bake_in_uvs.find(uv_name);
          if (buv != bake_in_uvs.end()) {
            // If we are to bake in a texture matrix, do so now.
            uv = uv * (*buv).second->get_transform();
          }

          bvert.set_texcoord(uv_name, LCAST(float, uv));
        }
        
        bprim.add_vertex(bvert);
      }
    }

    // Finally, if the primitive didn't have a color, and it didn't have
    // vertex color, make it white.
    if (!egg_prim->has_color() && !has_vert_color && !egg_false_color) {
      bprim.set_color(Colorf(1.0, 1.0, 1.0, 1.0));
    }

    _builder.add_prim(bucket, bprim);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_indexed_primitive
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggLoader::
make_indexed_primitive(EggPrimitive *egg_prim, PandaNode *parent,
                       const LMatrix4d *transform,
                       ComputedVerticesMaker &_comp_verts_maker) {
  BuilderBucket bucket;
  BakeInUVs bake_in_uvs;
  setup_bucket(bucket, bake_in_uvs, parent, egg_prim);
  if (bucket._hidden && egg_suppress_hidden) {
    // Eat this primitive.
    return;
  }

  LMatrix4d mat;

  if (transform != NULL) {
    mat = (*transform);
  } else {
    mat = egg_prim->get_vertex_to_node();
  }

  BuilderPrimI bprim;
  bprim.set_type(BPT_poly);
  if (egg_prim->is_of_type(EggLine::get_class_type())) {
    bprim.set_type(BPT_linestrip);
  } else if (egg_prim->is_of_type(EggPoint::get_class_type())) {
    bprim.set_type(BPT_point);
  }

  if (egg_prim->has_normal()) {
    // Define the transform space of the polygon normal.  This will be
    // the average of all the vertex transform spaces.
    _comp_verts_maker.begin_new_space();
    EggPrimitive::const_iterator vi;
    for (vi = egg_prim->begin(); vi != egg_prim->end(); ++vi) {
      EggVertex *egg_vert = *vi;
      _comp_verts_maker.add_vertex_joints(egg_vert, egg_prim);
    }
    _comp_verts_maker.mark_space();

    int nindex =
      _comp_verts_maker.add_normal(egg_prim->get_normal(),
                                   egg_prim->_dnormals, mat);

    bprim.set_normal(nindex);
  }

  if (egg_prim->has_color() && !egg_false_color) {
    int cindex =
      _comp_verts_maker.add_color(egg_prim->get_color(),
                                  egg_prim->_drgbas);
    bprim.set_color(cindex);
  }

  bool has_vert_color = true;
  EggPrimitive::const_iterator vi;
  for (vi = egg_prim->begin(); vi != egg_prim->end(); ++vi) {
    EggVertex *egg_vert = *vi;

    if (egg_vert->get_num_dimensions() != 3) {
      egg2pg_cat.error()
        << "Vertex " << egg_vert->get_pool()->get_name() 
        << ":" << egg_vert->get_index() << " has dimension " 
        << egg_vert->get_num_dimensions() << "\n";
    } else {
      // Set up the ComputedVerticesMaker for the coordinate space of
      // the vertex.
      _comp_verts_maker.begin_new_space();
      _comp_verts_maker.add_vertex_joints(egg_vert, egg_prim);
      _comp_verts_maker.mark_space();
      
      int vindex =
        _comp_verts_maker.add_vertex(egg_vert->get_pos3(),
                                     egg_vert->_dxyzs, mat);
      BuilderVertexI bvert(vindex);
      
      if (egg_vert->has_normal()) {
        int nindex =
          _comp_verts_maker.add_normal(egg_vert->get_normal(),
                                       egg_vert->_dnormals,
                                       mat);
        bvert.set_normal(nindex);
      }
      
      if (egg_vert->has_color() && !egg_false_color) {
        int cindex =
          _comp_verts_maker.add_color(egg_vert->get_color(),
                                      egg_vert->_drgbas);
        bvert.set_color(cindex);
      } else {
        // If any vertex doesn't have a color, we can't use any of the
        // vertex colors.
        has_vert_color = false;
      }

      EggVertex::const_uv_iterator ui;
      for (ui = egg_vert->uv_begin(); ui != egg_vert->uv_end(); ++ui) {
        EggVertexUV *uv_obj = (*ui);
        TexCoordd uv = uv_obj->get_uv();
        CPT(TexCoordName) uv_name;
        if (uv_obj->has_name()) {
          uv_name = TexCoordName::make(uv_obj->get_name());
        } else {
          uv_name = TexCoordName::get_default();
        }

        LMatrix3d mat = LMatrix3d::ident_mat();

        BakeInUVs::const_iterator buv = bake_in_uvs.find(uv_name);
        if (buv != bake_in_uvs.end()) {
          // If we are to bake in a texture matrix, do so now.
          mat = (*buv).second->get_transform();
        }
        
        int tindex =
          _comp_verts_maker.add_texcoord(uv_name, uv, uv_obj->_duvs, mat);
        bvert.set_texcoord(uv_name, tindex);
      }
      
      bprim.add_vertex(bvert);
    }
  }

  // Finally, if the primitive didn't have a color, and it didn't have
  // vertex color, make it white.
  if (!egg_prim->has_color() && !has_vert_color && !egg_false_color) {
    int cindex =
      _comp_verts_maker.add_color(Colorf(1.0, 1.0, 1.0, 1.0),
                                  EggMorphColorList());
    bprim.set_color(cindex);
  }

  // We have to set up the arrays after we have iterated through the
  // vertices, because iterating through the vertices might discover
  // additional texture coordinate sets that we didn't know about
  // before.
  bucket.set_coords(_comp_verts_maker._coords);
  bucket.set_normals(_comp_verts_maker._norms);
  bucket.set_colors(_comp_verts_maker._colors);

  ComputedVerticesMaker::TexCoords::const_iterator tci;
  for (tci = _comp_verts_maker._texcoords.begin();
       tci != _comp_verts_maker._texcoords.end();
       ++tci) {
    const TexCoordName *name = (*tci).first;
    bucket.set_texcoords(name, (*tci).second);
  }

  _builder.add_prim(bucket, bprim);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_nurbs_curve
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void EggLoader::
make_nurbs_curve(EggNurbsCurve *egg_curve, PandaNode *parent, 
                 const LMatrix4d &mat) {
  if (egg_load_old_curves) {
    // Make a NurbsCurve instead of a RopeNode (old interface).
    make_old_nurbs_curve(egg_curve, parent, mat);
    return;
  }

  assert(parent != NULL);
  assert(!parent->is_geom_node());

  PT(NurbsCurveEvaluator) nurbs = ::make_nurbs_curve(egg_curve, mat);
  if (nurbs == (NurbsCurveEvaluator *)NULL) {
    _error = true;
    return;
  }

  /*
  switch (egg_curve->get_curve_type()) {
  case EggCurve::CT_xyz:
    curve->set_curve_type(PCT_XYZ);
    break;

  case EggCurve::CT_hpr:
    curve->set_curve_type(PCT_HPR);
    break;

  case EggCurve::CT_t:
    curve->set_curve_type(PCT_T);
    break;

  default:
    break;
  }
  */

  PT(RopeNode) rope = new RopeNode(egg_curve->get_name());
  rope->set_curve(nurbs);

  // Respect the subdivision values in the egg file, if any.
  if (egg_curve->get_subdiv() != 0) {
    int subdiv_per_segment = 
      (int)((egg_curve->get_subdiv() + 0.5) / nurbs->get_num_segments());
    rope->set_num_subdiv(max(subdiv_per_segment, 1));
  }

  // Now get the attributes to apply to the rope.  We create a
  // BuilderBucket for this purpose, so we can call setup_bucket(),
  // but all we do with this bucket is immediately extract the state
  // from it.
  BuilderBucket bucket;
  BakeInUVs bake_in_uvs;
  setup_bucket(bucket, bake_in_uvs, parent, egg_curve);
  if (bucket._hidden && egg_suppress_hidden) {
    // Eat this primitive.
    return;
  }
  nassertv(bake_in_uvs.empty());

  rope->set_state(bucket._state);
  rope->set_uv_mode(RopeNode::UV_parametric);

  if (egg_curve->has_vertex_color()) {
    // If the curve had individual vertex color, enable it.
    rope->set_use_vertex_color(true);
  } else if (egg_curve->has_color()) {
    // Otherwise, if the curve has overall color, apply it.
    rope->set_attrib(ColorAttrib::make_flat(egg_curve->get_color()));
  }

  parent->add_child(rope);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_old_nurbs_curve
//       Access: Private
//  Description: This deprecated interface creates a NurbsCurve (or a
//               ClassicNurbsCurve) object for the EggNurbsCurve
//               entry.  It will eventually be removed in favor of the
//               above, which creates a RopeNode.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_old_nurbs_curve(EggNurbsCurve *egg_curve, PandaNode *parent,
                     const LMatrix4d &mat) {
  assert(parent != NULL);
  assert(!parent->is_geom_node());

  PT(ParametricCurve) curve;

  if (egg_load_classic_nurbs_curves) {
    curve = new ClassicNurbsCurve;
  } else {
    curve = new NurbsCurve;
  }

  NurbsCurveInterface *nurbs = curve->get_nurbs_interface();
  nassertv(nurbs != (NurbsCurveInterface *)NULL);

  if (egg_curve->get_order() < 1 || egg_curve->get_order() > 4) {
    egg2pg_cat.error()
      << "Invalid NURBSCurve order for " << egg_curve->get_name() << ": "
      << egg_curve->get_order() << "\n";
    _error = true;
    return;
  }

  nurbs->set_order(egg_curve->get_order());

  EggPrimitive::const_iterator pi;
  for (pi = egg_curve->begin(); pi != egg_curve->end(); ++pi) {
    nurbs->append_cv(LCAST(float, (*pi)->get_pos4() * mat));
  }

  int num_knots = egg_curve->get_num_knots();
  if (num_knots != nurbs->get_num_knots()) {
    egg2pg_cat.error()
      << "Invalid NURBSCurve number of knots for "
      << egg_curve->get_name() << ": got " << num_knots
      << " knots, expected " << nurbs->get_num_knots() << "\n";
    _error = true;
    return;
  }

  for (int i = 0; i < num_knots; i++) {
    nurbs->set_knot(i, egg_curve->get_knot(i));
  }

  switch (egg_curve->get_curve_type()) {
  case EggCurve::CT_xyz:
    curve->set_curve_type(PCT_XYZ);
    break;

  case EggCurve::CT_hpr:
    curve->set_curve_type(PCT_HPR);
    break;

  case EggCurve::CT_t:
    curve->set_curve_type(PCT_T);
    break;

  default:
    break;
  }
  curve->set_name(egg_curve->get_name());

  if (!curve->recompute()) {
    egg2pg_cat.error()
      << "Invalid NURBSCurve " << egg_curve->get_name() << "\n";
    _error = true;
    return;
  }

  parent->add_child(curve);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_nurbs_surface
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void EggLoader::
make_nurbs_surface(EggNurbsSurface *egg_surface, PandaNode *parent,
                   const LMatrix4d &mat) {
  assert(parent != NULL);
  assert(!parent->is_geom_node());

  PT(NurbsSurfaceEvaluator) nurbs = ::make_nurbs_surface(egg_surface, mat);
  if (nurbs == (NurbsSurfaceEvaluator *)NULL) {
    _error = true;
    return;
  }

  PT(SheetNode) sheet = new SheetNode(egg_surface->get_name());
  sheet->set_surface(nurbs);

  // Respect the subdivision values in the egg file, if any.
  if (egg_surface->get_u_subdiv() != 0) {
    int u_subdiv_per_segment = 
      (int)((egg_surface->get_u_subdiv() + 0.5) / nurbs->get_num_u_segments());
    sheet->set_num_u_subdiv(max(u_subdiv_per_segment, 1));
  }
  if (egg_surface->get_v_subdiv() != 0) {
    int v_subdiv_per_segment = 
      (int)((egg_surface->get_v_subdiv() + 0.5) / nurbs->get_num_v_segments());
    sheet->set_num_v_subdiv(max(v_subdiv_per_segment, 1));
  }

  // Now get the attributes to apply to the sheet.  We create a
  // BuilderBucket for this purpose, so we can call setup_bucket(),
  // but all we do with this bucket is immediately extract the state
  // from it.
  BuilderBucket bucket;
  BakeInUVs bake_in_uvs;
  setup_bucket(bucket, bake_in_uvs, parent, egg_surface);
  if (bucket._hidden && egg_suppress_hidden) {
    // Eat this primitive.
    return;
  }
  nassertv(bake_in_uvs.empty());

  sheet->set_state(bucket._state);

  if (egg_surface->has_vertex_color()) {
    // If the surface had individual vertex color, enable it.
    sheet->set_use_vertex_color(true);
  } else if (egg_surface->has_color()) {
    // Otherwise, if the surface has overall color, apply it.
    sheet->set_attrib(ColorAttrib::make_flat(egg_surface->get_color()));
  }

  parent->add_child(sheet);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::load_textures
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void EggLoader::
load_textures() {
  // First, collect all the textures that are referenced.
  EggTextureCollection tc;
  tc.find_used_textures(&_data);

  // Collapse the textures down by filename only.  Should we also
  // differentiate by attributes?  Maybe.
  EggTextureCollection::TextureReplacement replace;
  tc.collapse_equivalent_textures(EggTexture::E_complete_filename,
                                  replace);

  EggTextureCollection::iterator ti;
  for (ti = tc.begin(); ti != tc.end(); ++ti) {
    PT_EggTexture egg_tex = (*ti);

    TextureDef def;
    if (load_texture(def, egg_tex)) {
      // Now associate the pointers, so we'll be able to look up the
      // Texture pointer given an EggTexture pointer, later.
      _textures[egg_tex] = def;
    }
  }

  // Finally, associate all of the removed texture references back to
  // the same pointers as the others.
  EggTextureCollection::TextureReplacement::const_iterator ri;
  for (ri = replace.begin(); ri != replace.end(); ++ri) {
    PT_EggTexture orig = (*ri).first;
    PT_EggTexture repl = (*ri).second;

    TextureDef &def = _textures[orig];
    def = _textures[repl];
    def._egg_tex = orig;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::load_texture
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool EggLoader::
load_texture(TextureDef &def, const EggTexture *egg_tex) {
  // Check to see if we should reduce the number of channels in
  // the texture.
  int wanted_channels = 0;
  bool wanted_alpha = false;
  switch (egg_tex->get_format()) {
  case EggTexture::F_red:
  case EggTexture::F_green:
  case EggTexture::F_blue:
  case EggTexture::F_alpha:
  case EggTexture::F_luminance:
    wanted_channels = 1;
    wanted_alpha = false;
    break;

  case EggTexture::F_luminance_alpha:
  case EggTexture::F_luminance_alphamask:
    wanted_channels = 2;
    wanted_alpha = true;
    break;

  case EggTexture::F_rgb:
  case EggTexture::F_rgb12:
  case EggTexture::F_rgb8:
  case EggTexture::F_rgb5:
  case EggTexture::F_rgb332:
    wanted_channels = 3;
    wanted_alpha = false;
    break;

  case EggTexture::F_rgba:
  case EggTexture::F_rgbm:
  case EggTexture::F_rgba12:
  case EggTexture::F_rgba8:
  case EggTexture::F_rgba4:
  case EggTexture::F_rgba5:
    wanted_channels = 4;
    wanted_alpha = true;
    break;

  case EggTexture::F_unspecified:
    break;
  }

  Texture *tex;
  if (egg_tex->has_alpha_filename() && wanted_alpha) {
    tex = TexturePool::load_texture(egg_tex->get_fullpath(),
                                    egg_tex->get_alpha_fullpath(),
                                    wanted_channels,
                                    egg_tex->get_alpha_file_channel());
  } else {
    tex = TexturePool::load_texture(egg_tex->get_fullpath(),
                                    wanted_channels);
  }
  if (tex == (Texture *)NULL) {
    return false;
  }

  // Record the original filenames in the textures (as loaded from the
  // egg file).  These filenames will be written back to the bam file
  // if the bam file is written out.
  tex->set_filename(egg_tex->get_filename());
  if (egg_tex->has_alpha_filename() && wanted_alpha) {
    tex->set_alpha_filename(egg_tex->get_alpha_filename());
  }

  apply_texture_attributes(tex, egg_tex);

  // Make a texture stage for the texture.
  PT(TextureStage) stage = make_texture_stage(egg_tex);
  def._texture = DCAST(TextureAttrib, TextureAttrib::make())->add_on_stage(stage, tex);
  def._stage = stage;
  def._egg_tex = egg_tex;

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::apply_texture_attributes
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void EggLoader::
apply_texture_attributes(Texture *tex, const EggTexture *egg_tex) {
  switch (egg_tex->determine_wrap_u()) {
  case EggTexture::WM_repeat:
    tex->set_wrapu(Texture::WM_repeat);
    break;

  case EggTexture::WM_clamp:
    if (egg_ignore_clamp) {
      egg2pg_cat.warning()
        << "Ignoring clamp request\n";
      tex->set_wrapu(Texture::WM_repeat);
    } else {
      tex->set_wrapu(Texture::WM_clamp);
    }
    break;

  case EggTexture::WM_unspecified:
    break;

  default:
    egg2pg_cat.warning()
      << "Unexpected texture wrap flag: "
      << (int)egg_tex->determine_wrap_u() << "\n";
  }

  switch (egg_tex->determine_wrap_v()) {
  case EggTexture::WM_repeat:
    tex->set_wrapv(Texture::WM_repeat);
    break;

  case EggTexture::WM_clamp:
    if (egg_ignore_clamp) {
      egg2pg_cat.warning()
        << "Ignoring clamp request\n";
      tex->set_wrapv(Texture::WM_repeat);
    } else {
      tex->set_wrapv(Texture::WM_clamp);
    }
    break;

  case EggTexture::WM_unspecified:
    break;

  default:
    egg2pg_cat.warning()
      << "Unexpected texture wrap flag: "
      << (int)egg_tex->determine_wrap_v() << "\n";
  }

  switch (egg_tex->get_minfilter()) {
  case EggTexture::FT_nearest:
    tex->set_minfilter(Texture::FT_nearest);
    break;

  case EggTexture::FT_linear:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else {
      tex->set_minfilter(Texture::FT_linear);
    }
    break;

  case EggTexture::FT_nearest_mipmap_nearest:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2pg_cat.warning()
        << "Ignoring mipmap request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else {
      tex->set_minfilter(Texture::FT_nearest_mipmap_nearest);
    }
    break;

  case EggTexture::FT_linear_mipmap_nearest:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2pg_cat.warning()
        << "Ignoring mipmap request\n";
      tex->set_minfilter(Texture::FT_linear);
    } else {
      tex->set_minfilter(Texture::FT_linear_mipmap_nearest);
    }
    break;

  case EggTexture::FT_nearest_mipmap_linear:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2pg_cat.warning()
        << "Ignoring mipmap request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else {
      tex->set_minfilter(Texture::FT_nearest_mipmap_linear);
    }
    break;

  case EggTexture::FT_linear_mipmap_linear:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2pg_cat.warning()
        << "Ignoring mipmap request\n";
      tex->set_minfilter(Texture::FT_linear);
    } else {
      tex->set_minfilter(Texture::FT_linear_mipmap_linear);
    }
    break;

  case EggTexture::FT_unspecified:
    // Default is bilinear, unless egg_ignore_filters is specified.
    if (egg_ignore_filters) {
      tex->set_minfilter(Texture::FT_nearest);
    } else {
      tex->set_minfilter(Texture::FT_linear);
    }
  }

  switch (egg_tex->get_magfilter()) {
  case EggTexture::FT_nearest:
  case EggTexture::FT_nearest_mipmap_nearest:
  case EggTexture::FT_nearest_mipmap_linear:
    tex->set_magfilter(Texture::FT_nearest);
    break;

  case EggTexture::FT_linear:
  case EggTexture::FT_linear_mipmap_nearest:
  case EggTexture::FT_linear_mipmap_linear:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring magfilter request\n";
      tex->set_magfilter(Texture::FT_nearest);
    } else {
      tex->set_magfilter(Texture::FT_linear);
    }
    break;

  case EggTexture::FT_unspecified:
    // Default is bilinear, unless egg_ignore_filters is specified.
    if (egg_ignore_filters) {
      tex->set_magfilter(Texture::FT_nearest);
    } else {
      tex->set_magfilter(Texture::FT_linear);
    }
  }

  if (egg_tex->has_anisotropic_degree()) {
    tex->set_anisotropic_degree(egg_tex->get_anisotropic_degree());
  }

  if (tex->_pbuffer->get_num_components() == 1) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_red:
      tex->_pbuffer->set_format(PixelBuffer::F_red);
      break;
    case EggTexture::F_green:
      tex->_pbuffer->set_format(PixelBuffer::F_green);
      break;
    case EggTexture::F_blue:
      tex->_pbuffer->set_format(PixelBuffer::F_blue);
      break;
    case EggTexture::F_alpha:
      tex->_pbuffer->set_format(PixelBuffer::F_alpha);
      break;
    case EggTexture::F_luminance:
      tex->_pbuffer->set_format(PixelBuffer::F_luminance);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2pg_cat.warning()
        << "Ignoring inappropriate format " << egg_tex->get_format()
        << " for 1-component texture " << egg_tex->get_name() << "\n";
    }

  } else if (tex->_pbuffer->get_num_components() == 2) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_luminance_alpha:
      tex->_pbuffer->set_format(PixelBuffer::F_luminance_alpha);
      break;

    case EggTexture::F_luminance_alphamask:
      tex->_pbuffer->set_format(PixelBuffer::F_luminance_alphamask);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2pg_cat.warning()
        << "Ignoring inappropriate format " << egg_tex->get_format()
        << " for 2-component texture " << egg_tex->get_name() << "\n";
    }

  } else if (tex->_pbuffer->get_num_components() == 3) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_rgb:
      tex->_pbuffer->set_format(PixelBuffer::F_rgb);
      break;
    case EggTexture::F_rgb12:
      if (tex->_pbuffer->get_component_width() >= 2) {
        // Only do this if the component width supports it.
        tex->_pbuffer->set_format(PixelBuffer::F_rgb12);
      } else {
        egg2pg_cat.warning()
          << "Ignoring inappropriate format " << egg_tex->get_format()
          << " for 8-bit texture " << egg_tex->get_name() << "\n";
      }
      break;
    case EggTexture::F_rgb8:
    case EggTexture::F_rgba8:
      // We'll quietly accept RGBA8 for a 3-component texture, since
      // flt2egg generates these for 3-component as well as for
      // 4-component textures.
      tex->_pbuffer->set_format(PixelBuffer::F_rgb8);
      break;
    case EggTexture::F_rgb5:
      tex->_pbuffer->set_format(PixelBuffer::F_rgb5);
      break;
    case EggTexture::F_rgb332:
      tex->_pbuffer->set_format(PixelBuffer::F_rgb332);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2pg_cat.warning()
        << "Ignoring inappropriate format " << egg_tex->get_format()
        << " for 3-component texture " << egg_tex->get_name() << "\n";
    }

  } else if (tex->_pbuffer->get_num_components() == 4) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_rgba:
      tex->_pbuffer->set_format(PixelBuffer::F_rgba);
      break;
    case EggTexture::F_rgbm:
      tex->_pbuffer->set_format(PixelBuffer::F_rgbm);
      break;
    case EggTexture::F_rgba12:
      if (tex->_pbuffer->get_component_width() >= 2) {
        // Only do this if the component width supports it.
        tex->_pbuffer->set_format(PixelBuffer::F_rgba12);
      } else {
        egg2pg_cat.warning()
          << "Ignoring inappropriate format " << egg_tex->get_format()
          << " for 8-bit texture " << egg_tex->get_name() << "\n";
      }
      break;
    case EggTexture::F_rgba8:
      tex->_pbuffer->set_format(PixelBuffer::F_rgba8);
      break;
    case EggTexture::F_rgba4:
      tex->_pbuffer->set_format(PixelBuffer::F_rgba4);
      break;
    case EggTexture::F_rgba5:
      tex->_pbuffer->set_format(PixelBuffer::F_rgba5);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2pg_cat.warning()
        << "Ignoring inappropriate format " << egg_tex->get_format()
        << " for 4-component texture " << egg_tex->get_name() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_texture_stage
//       Access: Private
//  Description: Creates a TextureStage object suitable for rendering
//               the indicated texture.
////////////////////////////////////////////////////////////////////
PT(TextureStage) EggLoader::
make_texture_stage(const EggTexture *egg_tex) {
  // If the egg texture specifies any relevant TextureStage
  // properties, or if it is multitextured on top of anything else, it
  // gets its own texture stage; otherwise, it gets the default
  // texture stage.
  if (!egg_tex->has_stage_name() &&
      !egg_tex->has_uv_name() &&
      !egg_tex->has_color() && 
      (egg_tex->get_env_type() == EggTexture::ET_unspecified ||
       egg_tex->get_env_type() == EggTexture::ET_modulate) &&
      egg_tex->get_combine_mode(EggTexture::CC_rgb) == EggTexture::CM_unspecified &&
      egg_tex->get_combine_mode(EggTexture::CC_alpha) == EggTexture::CM_unspecified &&

      !egg_tex->has_priority() &&
      egg_tex->get_multitexture_sort() == 0) {
    return TextureStage::get_default();
  }

  PT(TextureStage) stage = new TextureStage(egg_tex->get_stage_name());

  switch (egg_tex->get_env_type()) {
  case EggTexture::ET_modulate:
    stage->set_mode(TextureStage::M_modulate);
    break;
    
  case EggTexture::ET_decal:
    stage->set_mode(TextureStage::M_decal);
    break;
    
  case EggTexture::ET_blend:
    stage->set_mode(TextureStage::M_blend);
    break;
    
  case EggTexture::ET_replace:
    stage->set_mode(TextureStage::M_replace);
    break;
    
  case EggTexture::ET_add:
    stage->set_mode(TextureStage::M_add);
    break;

  case EggTexture::ET_unspecified:
    break;
  }

  switch (egg_tex->get_combine_mode(EggTexture::CC_rgb)) {
  case EggTexture::CM_replace:
    stage->set_combine_rgb(get_combine_mode(egg_tex, EggTexture::CC_rgb),
                           get_combine_source(egg_tex, EggTexture::CC_rgb, 0),
                           get_combine_operand(egg_tex, EggTexture::CC_rgb, 0));
    break;

  case EggTexture::CM_modulate:
  case EggTexture::CM_add:
  case EggTexture::CM_add_signed:
  case EggTexture::CM_subtract:
  case EggTexture::CM_dot3_rgb:
  case EggTexture::CM_dot3_rgba:
    stage->set_combine_rgb(get_combine_mode(egg_tex, EggTexture::CC_rgb),
                           get_combine_source(egg_tex, EggTexture::CC_rgb, 0),
                           get_combine_operand(egg_tex, EggTexture::CC_rgb, 0),
                           get_combine_source(egg_tex, EggTexture::CC_rgb, 1),
                           get_combine_operand(egg_tex, EggTexture::CC_rgb, 1));
    break;

  case EggTexture::CM_interpolate:
    stage->set_combine_rgb(get_combine_mode(egg_tex, EggTexture::CC_rgb),
                           get_combine_source(egg_tex, EggTexture::CC_rgb, 0),
                           get_combine_operand(egg_tex, EggTexture::CC_rgb, 0),
                           get_combine_source(egg_tex, EggTexture::CC_rgb, 1),
                           get_combine_operand(egg_tex, EggTexture::CC_rgb, 1),
                           get_combine_source(egg_tex, EggTexture::CC_rgb, 2),
                           get_combine_operand(egg_tex, EggTexture::CC_rgb, 2));
    break;

  case EggTexture::CM_unspecified:
    break;
  }

  switch (egg_tex->get_combine_mode(EggTexture::CC_alpha)) {
  case EggTexture::CM_replace:
    stage->set_combine_alpha(get_combine_mode(egg_tex, EggTexture::CC_alpha),
                             get_combine_source(egg_tex, EggTexture::CC_alpha, 0),
                             get_combine_operand(egg_tex, EggTexture::CC_alpha, 0));
    break;

  case EggTexture::CM_modulate:
  case EggTexture::CM_add:
  case EggTexture::CM_add_signed:
  case EggTexture::CM_subtract:
    stage->set_combine_alpha(get_combine_mode(egg_tex, EggTexture::CC_alpha),
                             get_combine_source(egg_tex, EggTexture::CC_alpha, 0),
                             get_combine_operand(egg_tex, EggTexture::CC_alpha, 0),
                             get_combine_source(egg_tex, EggTexture::CC_alpha, 1),
                             get_combine_operand(egg_tex, EggTexture::CC_alpha, 1));
    break;
    
  case EggTexture::CM_interpolate:
    stage->set_combine_alpha(get_combine_mode(egg_tex, EggTexture::CC_alpha),
                             get_combine_source(egg_tex, EggTexture::CC_alpha, 0),
                             get_combine_operand(egg_tex, EggTexture::CC_alpha, 0),
                             get_combine_source(egg_tex, EggTexture::CC_alpha, 1),
                             get_combine_operand(egg_tex, EggTexture::CC_alpha, 1),
                             get_combine_source(egg_tex, EggTexture::CC_alpha, 2),
                             get_combine_operand(egg_tex, EggTexture::CC_alpha, 2));
    break;

  case EggTexture::CM_unspecified:
  case EggTexture::CM_dot3_rgb:
  case EggTexture::CM_dot3_rgba:
    break;
  }


  if (egg_tex->has_uv_name() && !egg_tex->get_uv_name().empty()) {
    CPT(TexCoordName) name = 
      TexCoordName::make(egg_tex->get_uv_name());
    stage->set_texcoord_name(name);
  }

  stage->set_sort(egg_tex->get_multitexture_sort() * 10);

  if (egg_tex->has_priority()) {
    stage->set_sort(egg_tex->get_priority());
  }

  if (egg_tex->has_color()) {
    stage->set_color(egg_tex->get_color());
  }

  return stage;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::get_material_attrib
//       Access: Private
//  Description: Returns a RenderAttrib suitable for enabling the
//               material indicated by the given EggMaterial, and with
//               the indicated backface flag.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) EggLoader::
get_material_attrib(const EggMaterial *egg_mat, bool bface) {
  Materials &materials = bface ? _materials_bface : _materials;

  // First, check whether we've seen this material before.
  Materials::const_iterator mi;
  mi = materials.find(egg_mat);
  if (mi != materials.end()) {
    return (*mi).second;
  }

  // Ok, this is the first time we've seen this particular
  // EggMaterial.  Create a new Material that matches it.
  PT(Material) mat = new Material;
  if (egg_mat->has_diff()) {
    mat->set_diffuse(egg_mat->get_diff());
    // By default, ambient is the same as diffuse, if diffuse is
    // specified but ambient is not.
    mat->set_ambient(egg_mat->get_diff());
  }
  if (egg_mat->has_amb()) {
    mat->set_ambient(egg_mat->get_amb());
  }
  if (egg_mat->has_emit()) {
    mat->set_emission(egg_mat->get_emit());
  }
  if (egg_mat->has_spec()) {
    mat->set_specular(egg_mat->get_spec());
  }
  if (egg_mat->has_shininess()) {
    mat->set_shininess(egg_mat->get_shininess());
  }
  if (egg_mat->has_local()) {
    mat->set_local(egg_mat->get_local());
  }

  mat->set_twoside(bface);

  // Now get a global Material pointer, shared with other models.
  const Material *shared_mat = MaterialPool::get_material(mat);

  // And create a MaterialAttrib for this Material.
  CPT(RenderAttrib) mt = MaterialAttrib::make(shared_mat);
  materials.insert(Materials::value_type(egg_mat, mt));

  return mt;
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::setup_bucket
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void EggLoader::
setup_bucket(BuilderBucket &bucket, EggLoader::BakeInUVs &bake_in_uvs,
             PandaNode *parent, EggPrimitive *egg_prim) {
  bucket._node = parent;
  bucket._mesh = egg_mesh;
  bucket._retesselate_coplanar = egg_retesselate_coplanar;
  bucket._unroll_fans = egg_unroll_fans;
  bucket._show_tstrips = egg_show_tstrips;
  bucket._show_qsheets = egg_show_qsheets;
  bucket._show_quads = egg_show_quads;
  bucket._show_normals = egg_show_normals;
  bucket._normal_scale = egg_normal_scale;
  bucket._subdivide_polys = egg_subdivide_polys;
  bucket._consider_fans = egg_consider_fans;
  bucket._max_tfan_angle = egg_max_tfan_angle;
  bucket._min_tfan_tris = egg_min_tfan_tris;
  bucket._coplanar_threshold = egg_coplanar_threshold;

  // If a primitive has a name that does not begin with a digit, it
  // should be used to group primitives together--i.e. each primitive
  // with the same name gets placed into the same GeomNode.  However,
  // if a prim's name begins with a digit, just ignore it.
  if (egg_prim->has_name() && !isdigit(egg_prim->get_name()[0])) {
    bucket.set_name(egg_prim->get_name());
  }

  // Assign the appropriate properties to the bucket.

  // The various EggRenderMode properties can be defined directly at
  // the primitive, at a group above the primitive, or an a texture
  // applied to the primitive.  The EggNode::determine_*() functions
  // can find the right pointer to the level at which this is actually
  // defined for a given primitive.
  EggRenderMode::AlphaMode am = EggRenderMode::AM_unspecified;
  EggRenderMode::DepthWriteMode dwm = EggRenderMode::DWM_unspecified;
  EggRenderMode::DepthTestMode dtm = EggRenderMode::DTM_unspecified;
  EggRenderMode::VisibilityMode vm = EggRenderMode::VM_unspecified;
  bool implicit_alpha = false;
  bool has_draw_order = false;
  int draw_order = 0;
  bool has_bin = false;
  string bin;

  EggRenderMode *render_mode;
  render_mode = egg_prim->determine_alpha_mode();
  if (render_mode != (EggRenderMode *)NULL) {
    am = render_mode->get_alpha_mode();
  }
  render_mode = egg_prim->determine_depth_write_mode();
  if (render_mode != (EggRenderMode *)NULL) {
    dwm = render_mode->get_depth_write_mode();
  }
  render_mode = egg_prim->determine_depth_test_mode();
  if (render_mode != (EggRenderMode *)NULL) {
    dtm = render_mode->get_depth_test_mode();
  }
  render_mode = egg_prim->determine_visibility_mode();
  if (render_mode != (EggRenderMode *)NULL) {
    vm = render_mode->get_visibility_mode();
  }
  render_mode = egg_prim->determine_draw_order();
  if (render_mode != (EggRenderMode *)NULL) {
    has_draw_order = true;
    draw_order = render_mode->get_draw_order();
  }
  render_mode = egg_prim->determine_bin();
  if (render_mode != (EggRenderMode *)NULL) {
    has_bin = true;
    bin = render_mode->get_bin();
  }

  bucket.add_attrib(TextureAttrib::make_off());
  int num_textures = egg_prim->get_num_textures();
  CPT(RenderAttrib) texture_attrib = NULL;
  CPT(RenderAttrib) tex_gen_attrib = NULL;
  CPT(RenderAttrib) tex_mat_attrib = NULL;
  TexMats tex_mats;

  for (int i = 0; i < num_textures; i++) {
    PT_EggTexture egg_tex = egg_prim->get_texture(i);

    const TextureDef &def = _textures[egg_tex];
    if (def._texture != (const RenderAttrib *)NULL) {
      if (texture_attrib == (RenderAttrib *)NULL) {
        texture_attrib = def._texture;
      } else {
        texture_attrib = texture_attrib->compose(def._texture);
      }

      // If neither the primitive nor the texture specified an alpha
      // mode, assume it should be alpha'ed if the texture has an
      // alpha channel (unless the texture environment type is one
      // that doesn't apply its alpha to the result).
      if (am == EggRenderMode::AM_unspecified) {
        const TextureAttrib *tex_attrib = DCAST(TextureAttrib, def._texture);
        Texture *tex = tex_attrib->get_texture();
        nassertv(tex != (Texture *)NULL);
        int num_components = tex->_pbuffer->get_num_components();
        if (egg_tex->has_alpha_channel(num_components)) {
          switch (egg_tex->get_env_type()) {
          case EggTexture::ET_decal:
          case EggTexture::ET_add:
            break;

          default:
            implicit_alpha = true;
          }
        }
      }

      // Check for a texgen attrib.
      bool has_tex_gen = false;
      if (egg_tex->get_tex_gen() != EggTexture::TG_unspecified) {
        has_tex_gen = true;
        if (tex_gen_attrib == (const RenderAttrib *)NULL) {
          tex_gen_attrib = TexGenAttrib::make();
        }
        tex_gen_attrib = DCAST(TexGenAttrib, tex_gen_attrib)->
          add_stage(def._stage, get_tex_gen(egg_tex));
      }

      // Record the texture's associated texture matrix, so we can see
      // if we can safely bake it into the UV's.  (We need to get the
      // complete list of textures that share this same set of UV's
      // per each unique texture matrix.  Whew!)
      CPT(TexCoordName) uv_name;
      if (egg_tex->has_uv_name()) {
        uv_name = TexCoordName::make(egg_tex->get_uv_name());
      } else {
        uv_name = TexCoordName::get_default();
      }

      if (has_tex_gen) {
        // If the texture has a texgen mode, we will always apply its
        // texture transform, never bake it in.  In fact, we don't
        // even care about its UV's in this case, since we won't be
        // using them.
        tex_mat_attrib = apply_tex_mat(tex_mat_attrib, def._stage, egg_tex);

      } else {
        // Otherwise, we need to record that there is at least one
        // texture on this particular UV name and with this particular
        // texture matrix.  If there are no other textures, or if all
        // of the other textures use the same texture matrix, then
        // tex_mats[uv_name].size() will remain 1 (which tells us we
        // can bake in the texture matrix to the UV's).  On the other
        // hand, if there is another texture on the same uv name but
        // with a different transform, it will increase
        // tex_mats[uv_name].size() to at least 2, indicating we can't
        // bake in the texture matrix.
        tex_mats[uv_name][egg_tex->get_transform()].push_back(&def);
      }
    }
  }

  // These parametric primitive types can't have their UV's baked in,
  // so if we have one of these we always need to apply the texture
  // matrix as a separate attribute, regardless of how many textures
  // share the particular UV set.
  bool needs_tex_mat = (egg_prim->is_of_type(EggCurve::get_class_type()) ||
                        egg_prim->is_of_type(EggSurface::get_class_type()));

  // Now that we've visited all of the textures in the above loop, we
  // can go back and see how many of them share the same UV name and
  // texture matrix.
  TexMats::const_iterator tmi;
  for (tmi = tex_mats.begin(); tmi != tex_mats.end(); ++tmi) {
    const TexCoordName *uv_name = (*tmi).first;
    const TexMatTransforms &tmt = (*tmi).second;

    if (tmt.size() == 1 && !needs_tex_mat) {
      // Only one unique transform sharing this set of UV's.  We can
      // bake in the transform!
      const TexMatTextures &tmtex = (*tmt.begin()).second;

      // The first EggTexture on the list is sufficient, since we know
      // they all have the same transform.
      nassertv(!tmtex.empty());
      TexMatTextures::const_iterator tmtexi = tmtex.begin();
      const EggTexture *egg_tex = (*tmtexi)->_egg_tex;
      if (egg_tex->has_transform()) {
        // If there's no transform, it's an identity matrix; don't
        // bother recording it.  Of course, it would do no harm to
        // record it if we felt like it.
        bake_in_uvs[uv_name] = egg_tex;
      }

    } else {
      // Multiple transforms on this UV set, or a geometry type that
      // doesn't support baking in UV's.  We have to apply the
      // texture matrix to each stage.
      TexMatTransforms::const_iterator tmti;
      for (tmti = tmt.begin(); tmti != tmt.end(); ++tmti) {
        const TexMatTextures &tmtex = (*tmti).second;
        TexMatTextures::const_iterator tmtexi;
        for (tmtexi = tmtex.begin(); tmtexi != tmtex.end(); ++tmtexi) {
          const EggTexture *egg_tex = (*tmtexi)->_egg_tex;
          TextureStage *stage = (*tmtexi)->_stage;
          
          tex_mat_attrib = apply_tex_mat(tex_mat_attrib, stage, egg_tex);
        }
      }
    }
  }

  if (texture_attrib != (RenderAttrib *)NULL) {
    bucket.add_attrib(texture_attrib);
  }

  if (tex_gen_attrib != (RenderAttrib *)NULL) {
    bucket.add_attrib(tex_gen_attrib);
  }

  if (tex_mat_attrib != (RenderAttrib *)NULL) {
    bucket.add_attrib(tex_mat_attrib);
  }

  if (egg_prim->has_material()) {
    CPT(RenderAttrib) mt =
      get_material_attrib(egg_prim->get_material(),
                          egg_prim->get_bface_flag());
    bucket.add_attrib(mt);
  }


  // Also check the color of the primitive to see if we should assume
  // alpha based on the alpha values specified in the egg file.
  if (am == EggRenderMode::AM_unspecified) {
    if (egg_prim->has_color()) {
      if (egg_prim->get_color()[3] != 1.0) {
        implicit_alpha = true;
      }
    }
    EggPrimitive::const_iterator vi;
    for (vi = egg_prim->begin();
         !implicit_alpha && vi != egg_prim->end();
         ++vi) {
      if ((*vi)->has_color()) {
        if ((*vi)->get_color()[3] != 1.0) {
          implicit_alpha = true;
        }
      }
    }

    if (implicit_alpha) {
      am = EggRenderMode::AM_on;
    }
  }

  if (am == EggRenderMode::AM_on) {
    // Alpha type "on" means to get the default transparency type.
    am = egg_alpha_mode;
  }

  switch (am) {
  case EggRenderMode::AM_on:
  case EggRenderMode::AM_blend:
    bucket.add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    break;

  case EggRenderMode::AM_blend_no_occlude:
    bucket.add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    bucket.add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
    break;

  case EggRenderMode::AM_ms:
    bucket.add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_multisample));
    break;

  case EggRenderMode::AM_ms_mask:
    bucket.add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_multisample_mask));
    break;

  case EggRenderMode::AM_binary:
    bucket.add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_binary));
    break;

  case EggRenderMode::AM_dual:
    bucket.add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_dual));
    break;

  default:
    break;
  }

  switch (dwm) {
  case EggRenderMode::DWM_on:
    bucket.add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_on));
    break;

  case EggRenderMode::DWM_off:
    bucket.add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
    break;

  default:
    break;
  }

  switch (dtm) {
  case EggRenderMode::DTM_on:
    bucket.add_attrib(DepthTestAttrib::make(DepthTestAttrib::M_less));
    break;

  case EggRenderMode::DTM_off:
    bucket.add_attrib(DepthTestAttrib::make(DepthTestAttrib::M_none));
    break;

  default:
    break;
  }

  switch (vm) {
  case EggRenderMode::VM_hidden:
    bucket._hidden = true;
    break;

  case EggRenderMode::VM_normal:
  default:
    break;
  }

  if (has_bin) {
    bucket.add_attrib(CullBinAttrib::make(bin, draw_order));

  } else if (has_draw_order) {
    bucket.add_attrib(CullBinAttrib::make("fixed", draw_order));
  }
 

  if (egg_prim->get_bface_flag()) {
    // The primitive is marked with backface culling disabled--we want
    // to see both sides.
    bucket.add_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none));
  }
}



////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode *EggLoader::
make_node(EggNode *egg_node, PandaNode *parent) {
  if (egg_node->is_of_type(EggPrimitive::get_class_type())) {
    return make_node(DCAST(EggPrimitive, egg_node), parent);
  } else if (egg_node->is_of_type(EggBin::get_class_type())) {
    return make_node(DCAST(EggBin, egg_node), parent);
  } else if (egg_node->is_of_type(EggGroup::get_class_type())) {
    return make_node(DCAST(EggGroup, egg_node), parent);
  } else if (egg_node->is_of_type(EggTable::get_class_type())) {
    return make_node(DCAST(EggTable, egg_node), parent);
  } else if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    return make_node(DCAST(EggGroupNode, egg_node), parent);
  }

  return (PandaNode *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggPrimitive)
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode *EggLoader::
make_node(EggPrimitive *egg_prim, PandaNode *parent) {
  assert(parent != NULL);
  assert(!parent->is_of_type(GeomNode::get_class_type()));

  if (egg_prim->cleanup()) {
    if (parent->is_of_type(SelectiveChildNode::get_class_type())) {
      // If we're putting a primitive under a SelectiveChildNode of
      // some kind, its exact position within the group is relevant,
      // so we need to create a placeholder now.
      PandaNode *group = new PandaNode(egg_prim->get_name());
      parent->add_child(group);
      make_nonindexed_primitive(egg_prim, group);
      return group;
    }

    // Otherwise, we don't really care what the position of this
    // primitive is within its parent's list of children, and in fact
    // we want to allow it to be combined with other polygons added to
    // the same parent.
    make_nonindexed_primitive(egg_prim, parent);
  }
  return (PandaNode *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggBin)
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode *EggLoader::
make_node(EggBin *egg_bin, PandaNode *parent) {
  // Presently, an EggBin can only mean an LOD node (i.e. a parent of
  // one or more EggGroups with LOD specifications).  Later it might
  // mean other things as well.

  nassertr((EggBinner::BinNumber)egg_bin->get_bin_number() == EggBinner::BN_lod, NULL);
  LODNode *lod_node = new LODNode(egg_bin->get_name());

  pvector<LODInstance> instances;

  EggGroup::const_iterator ci;
  for (ci = egg_bin->begin(); ci != egg_bin->end(); ++ci) {
    LODInstance instance(*ci);
    instances.push_back(instance);
  }

  // Now that we've created all of our children, put them in the
  // proper order and tell the LOD node about them.
  sort(instances.begin(), instances.end());

  if (!instances.empty()) {
    // Set up the LOD node's center.  All of the children should have
    // the same center, because that's how we binned them.
    lod_node->set_center(LCAST(float, instances[0]._d->_center));
  }

  for (size_t i = 0; i < instances.size(); i++) {
    // Create the children in the proper order within the scene graph.
    const LODInstance &instance = instances[i];
    make_node(instance._egg_node, lod_node);

    // All of the children should have the same center, because that's
    // how we binned them.
    nassertr(lod_node->get_center().almost_equal
             (LCAST(float, instance._d->_center), 0.01), NULL);

    // Tell the LOD node about this child's switching distances.
    lod_node->add_switch(instance._d->_switch_in, instance._d->_switch_out);
  }

  return create_group_arc(egg_bin, parent, lod_node);
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggGroup)
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode *EggLoader::
make_node(EggGroup *egg_group, PandaNode *parent) {
  PT(PandaNode) node = NULL;

  if (egg_group->get_num_object_types() != 0) {
    pset<string> expanded;
    pvector<string> expanded_history;
    if (!expand_object_types(egg_group, expanded, expanded_history)) {
      return NULL;
    }
  }

  if (egg_group->get_dart_type() != EggGroup::DT_none) {
    // A group with the <Dart> flag set means to create a character.
    CharacterMaker char_maker(egg_group, *this);
    node = char_maker.make_node();

  } else if (egg_group->get_cs_type() != EggGroup::CST_none) {
    // A collision group: create collision geometry.
    node = new CollisionNode(egg_group->get_name());

    make_collision_solids(egg_group, egg_group, (CollisionNode *)node.p());
    if ((egg_group->get_collide_flags() & EggGroup::CF_keep) != 0) {
      // If we also specified to keep the geometry, continue the
      // traversal.
      EggGroup::const_iterator ci;
      for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
        make_node(*ci, parent);
      }
    }

    node = create_group_arc(egg_group, parent, node);

    if (!egg_show_collision_solids) {
      node->set_draw_mask(DrawMask::all_off());
    }
    return node;

  } else if (egg_group->get_switch_flag()) {
    if (egg_group->get_switch_fps() != 0.0) {
      // Create a sequence node.
      node = new SequenceNode(egg_group->get_switch_fps(), 
                              egg_group->get_name());
    } else {
      // Create a switch node.
      node = new SwitchNode(egg_group->get_name());
    }
      
    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }

  } else if (egg_group->get_model_flag() || 
             egg_group->get_dcs_type() != EggGroup::DC_none) {
    // A model or DCS flag; create a model node.
    node = new ModelNode(egg_group->get_name());
    switch (egg_group->get_dcs_type()) {
    case EggGroup::DC_net:
      DCAST(ModelNode, node)->set_preserve_transform(ModelNode::PT_net);
      break;

    case EggGroup::DC_local:
    case EggGroup::DC_default:
      DCAST(ModelNode, node)->set_preserve_transform(ModelNode::PT_local);
      break;

    case EggGroup::DC_none:
      break;
    }

    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }

  } else if (egg_group->get_portal_flag()) {
    // Create a portal instead of a regular polyset.  Scan the
    // children of this node looking for a polygon, similar to the
    // collision polygon case, above.
    PortalNode *pnode = new PortalNode(egg_group->get_name());
    node = pnode;

    set_portal_polygon(egg_group, pnode);
    if (pnode->get_num_vertices() == 0) {
      egg2pg_cat.warning()
        << "Portal " << egg_group->get_name() << " has no vertices!\n";
	}

  } else if (egg_group->get_polylight_flag()) {
    // Create a polylight instead of a regular polyset.
    // use make_sphere to get the center, radius and color
    //egg2pg_cat.debug() << "polylight node\n";
    LPoint3f center;
	Colorf color;
	float radius;
	
	if(!make_sphere(egg_group,center,radius,color)) {
      egg2pg_cat.warning()
        << "Polylight " << egg_group->get_name() << " make_sphere failed!\n";
	}
	PolylightNode *pnode = new PolylightNode(egg_group->get_name());
    pnode->set_pos(center);
    pnode->set_color(color);
    pnode->set_radius(radius);
    node = pnode;
    
  } else {
    // A normal group; just create a normal node, and traverse.
    node = new PandaNode(egg_group->get_name());

    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }
  }

  if (node == (PandaNode *)NULL) {
    return NULL;
  }
  return create_group_arc(egg_group, parent, node);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::create_group_arc
//       Access: Private
//  Description: Creates the arc parenting a new group to the scene
//               graph, and applies any relevant attribs to the
//               arc according to the EggGroup node that inspired the
//               group.
////////////////////////////////////////////////////////////////////
PandaNode *EggLoader::
create_group_arc(EggGroup *egg_group, PandaNode *parent, PandaNode *node) {
  parent->add_child(node);

  // If the group had a transform, apply it to the node.
  if (egg_group->has_transform()) {
    CPT(TransformState) transform = make_transform(egg_group);
    node->set_transform(transform);
    node->set_prev_transform(transform);
  }

  // If the group has a billboard flag, apply that.
  switch (egg_group->get_billboard_type()) {
  case EggGroup::BT_point_camera_relative:
    node->set_effect(BillboardEffect::make_point_eye());
    break;

  case EggGroup::BT_point_world_relative:
    node->set_effect(BillboardEffect::make_point_world());
    break;

  case EggGroup::BT_axis:
    node->set_effect(BillboardEffect::make_axis());
    break;

  case EggGroup::BT_none:
    break;
  }

  if (egg_group->get_decal_flag()) {
    if (egg_ignore_decals) {
      egg2pg_cat.error()
        << "Ignoring decal flag on " << egg_group->get_name() << "\n";
      _error = true;
    }

    // If the group has the "decal" flag set, it means that all of the
    // descendant groups will be decaled onto the geometry within
    // this group.  This means we'll need to reparent things a bit
    // afterward.
    _decals.insert(node);
  }

  // Copy all the tags from the group onto the node.
  EggGroup::TagData::const_iterator ti;
  for (ti = egg_group->tag_begin(); ti != egg_group->tag_end(); ++ti) {
    node->set_tag((*ti).first, (*ti).second);
  }

  // If the group specified some property that should propagate down
  // to the leaves, we have to remember this node and apply the
  // property later, after we've created the actual geometry.
  DeferredNodeProperty def;
  if (egg_group->has_collide_mask()) {
    def._from_collide_mask = egg_group->get_collide_mask();
    def._into_collide_mask = egg_group->get_collide_mask();
    def._flags |=
      DeferredNodeProperty::F_has_from_collide_mask |
      DeferredNodeProperty::F_has_into_collide_mask;
  }
  if (egg_group->has_from_collide_mask()) {
    def._from_collide_mask = egg_group->get_from_collide_mask();
    def._flags |= DeferredNodeProperty::F_has_from_collide_mask;
  }
  if (egg_group->has_into_collide_mask()) {
    def._into_collide_mask = egg_group->get_into_collide_mask();
    def._flags |= DeferredNodeProperty::F_has_into_collide_mask;
  }

  if (def._flags != 0) {
    _deferred_nodes[node] = def;
  }

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggTable)
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode *EggLoader::
make_node(EggTable *egg_table, PandaNode *parent) {
  if (egg_table->get_table_type() != EggTable::TT_bundle) {
    // We only do anything with bundles.  Isolated tables are treated
    // as ordinary groups.
    return make_node(DCAST(EggGroupNode, egg_table), parent);
  }

  // It's an actual bundle, so make an AnimBundle from it and its
  // descendants.
  AnimBundleMaker bundle_maker(egg_table);
  AnimBundleNode *node = bundle_maker.make_node();
  parent->add_child(node);
  return node;
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggGroupNode)
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PandaNode *EggLoader::
make_node(EggGroupNode *egg_group, PandaNode *parent) {
  PandaNode *node = new PandaNode(egg_group->get_name());

  EggGroupNode::const_iterator ci;
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    make_node(*ci, node);
  }

  parent->add_child(node);
  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::set_portal_polygon
//       Access: Private
//  Description: Defines the PortalNode from the first polygon found
//               within this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
set_portal_polygon(EggGroup *egg_group, PortalNode *pnode) {
  pnode->clear_vertices();

  EggPolygon *poly = find_first_polygon(egg_group);
  if (poly != (EggPolygon *)NULL) {
    LMatrix4d mat = poly->get_vertex_to_node();

    EggPolygon::const_iterator vi;
    for (vi = poly->begin(); vi != poly->end(); ++vi) {
      Vertexd vert = (*vi)->get_pos3() * mat;
      pnode->add_vertex(LCAST(float, vert));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::find_first_polygon
//       Access: Private
//  Description: Returns the first EggPolygon found at or below the
//               indicated node.
////////////////////////////////////////////////////////////////////
EggPolygon *EggLoader::
find_first_polygon(EggGroup *egg_group) {
  // Does this group have any polygons?
  EggGroup::const_iterator ci;
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
      // Yes!  Return the polygon.
      return DCAST(EggPolygon, (*ci));
    }
  }

  // Well, the group had no polygons; look for a child group that
  // does.
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggGroup::get_class_type())) {
      EggGroup *child_group = DCAST(EggGroup, *ci);
      EggPolygon *found = find_first_polygon(child_group);
      if (found != NULL) {
        return found;
      }
    }
  }

  // We got nothing.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_sphere
//       Access: Private
//  Description: Creates a single generic Sphere corresponding
//               to the polygons associated with this group.
//               This sphere is used by make_collision_sphere and
//               Polylight sphere. It could be used for other spheres.
////////////////////////////////////////////////////////////////////
bool EggLoader::
make_sphere(EggGroup *egg_group, LPoint3f &center, float &radius, Colorf &color) {
  bool success=false;
  EggGroup *geom_group = find_collision_geometry(egg_group);
  if (geom_group != (EggGroup *)NULL) {
    // Collect all of the vertices.
    pset<EggVertex *> vertices;

    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPrimitive::get_class_type())) {
        EggPrimitive *prim = DCAST(EggPrimitive, *ci);
        EggPrimitive::const_iterator pi;
        for (pi = prim->begin(); pi != prim->end(); ++pi) {
          vertices.insert(*pi);
        }
      }
    }

    // Now average together all of the vertices to get a center.
    int num_vertices = 0;
    LPoint3d d_center(0.0, 0.0, 0.0);
    pset<EggVertex *>::const_iterator vi;

    for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
      EggVertex *vtx = (*vi);
      d_center += vtx->get_pos3();
      num_vertices++;
    }

    if (num_vertices > 0) {
      d_center /= (double)num_vertices;
	  //egg2pg_cat.debug() << "make_sphere d_center: " << d_center << "\n";

      LMatrix4d mat = egg_group->get_vertex_to_node();
      d_center = d_center * mat;

      // And the furthest vertex determines the radius.
      double radius2 = 0.0;
      for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
        EggVertex *vtx = (*vi);
        LPoint3d p3 = vtx->get_pos3();
		LVector3d v = p3 * mat - d_center;
        radius2 = max(radius2, v.length_squared());
      }

      center = LCAST(float,d_center);
      radius = sqrtf(radius2);
	  //egg2pg_cat.debug() << "make_sphere radius: " << radius << "\n";
	  vi = vertices.begin();
	  EggVertex *clr_vtx = (*vi);
	  if (clr_vtx->has_color()) {
	    color = clr_vtx->get_color();
	  }
	  else {
	    color = Colorf(1.0,1.0,1.0,1.0);
	  }
      success = true;

    }
  }
  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_solids
//       Access: Private
//  Description: Creates CollisionSolids corresponding to the
//               collision geometry indicated at the given node and
//               below.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_solids(EggGroup *start_group, EggGroup *egg_group,
                      CollisionNode *cnode) {
  if (egg_group->get_cs_type() != EggGroup::CST_none) {
    start_group = egg_group;
  }

  switch (start_group->get_cs_type()) {
  case EggGroup::CST_none:
    // No collision flags; do nothing.  Don't even traverse further.
    return;

  case EggGroup::CST_plane:
    make_collision_plane(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_polygon:
    make_collision_polygon(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_polyset:
    make_collision_polyset(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_sphere:
    make_collision_sphere(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_tube:
    make_collision_tube(egg_group, cnode, start_group->get_collide_flags());
    break;
  }

  if ((start_group->get_collide_flags() & EggGroup::CF_descend) != 0) {
    // Now pick up everything below.
    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggGroup::get_class_type())) {
        make_collision_solids(start_group, DCAST(EggGroup, *ci), cnode);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_plane
//       Access: Private
//  Description: Creates a single CollisionPlane corresponding
//               to the first polygon associated with this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_plane(EggGroup *egg_group, CollisionNode *cnode,
                     EggGroup::CollideFlags flags) {
  EggGroup *geom_group = find_collision_geometry(egg_group);
  if (geom_group != (EggGroup *)NULL) {
    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
        CollisionPlane *csplane =
          create_collision_plane(DCAST(EggPolygon, *ci), egg_group);
        if (csplane != (CollisionPlane *)NULL) {
          apply_collision_flags(csplane, flags);
          cnode->add_solid(csplane);
          return;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_polygon
//       Access: Private
//  Description: Creates a single CollisionPolygon corresponding
//               to the first polygon associated with this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_polygon(EggGroup *egg_group, CollisionNode *cnode,
                       EggGroup::CollideFlags flags) {

  EggGroup *geom_group = find_collision_geometry(egg_group);
  if (geom_group != (EggGroup *)NULL) {
    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
        create_collision_polygons(cnode, DCAST(EggPolygon, *ci),
                                  egg_group, flags);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_polyset
//       Access: Private
//  Description: Creates a series of CollisionPolygons corresponding
//               to the polygons associated with this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_polyset(EggGroup *egg_group, CollisionNode *cnode,
                       EggGroup::CollideFlags flags) {
  EggGroup *geom_group = find_collision_geometry(egg_group);
  if (geom_group != (EggGroup *)NULL) {
    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
        create_collision_polygons(cnode, DCAST(EggPolygon, *ci),
                                  egg_group, flags);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_sphere
//       Access: Private
//  Description: Creates a single CollisionSphere corresponding
//               to the polygons associated with this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_sphere(EggGroup *egg_group, CollisionNode *cnode,
                      EggGroup::CollideFlags flags) {
  LPoint3f center;
  float radius;
  Colorf dummycolor;
  if (make_sphere(egg_group, center, radius, dummycolor)) {
    CollisionSphere *cssphere =
      new CollisionSphere(center, radius);
    apply_collision_flags(cssphere, flags);
    cnode->add_solid(cssphere);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_tube
//       Access: Private
//  Description: Creates a single CollisionTube corresponding
//               to the polygons associated with this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_tube(EggGroup *egg_group, CollisionNode *cnode,
                    EggGroup::CollideFlags flags) {
  EggGroup *geom_group = find_collision_geometry(egg_group);
  if (geom_group != (EggGroup *)NULL) {
    // Collect all of the vertices.
    pset<EggVertex *> vertices;

    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPrimitive::get_class_type())) {
        EggPrimitive *prim = DCAST(EggPrimitive, *ci);
        EggPrimitive::const_iterator pi;
        for (pi = prim->begin(); pi != prim->end(); ++pi) {
          vertices.insert(*pi);
        }
      }
    }

    // Now store the 3-d values in a vector for convenient access (and
    // also determine the centroid).  We compute this in node space.
    size_t num_vertices = vertices.size();
    if (num_vertices != 0) {
      LMatrix4d mat = egg_group->get_vertex_to_node();
      pvector<LPoint3d> vpos;
      vpos.reserve(num_vertices);
      
      LPoint3d center(0.0, 0.0, 0.0);
      pset<EggVertex *>::const_iterator vi;
      for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
        EggVertex *vtx = (*vi);
        LPoint3d pos = vtx->get_pos3() * mat;
        vpos.push_back(pos);
        center += pos;
      }
      center /= (double)num_vertices;

      // Now that we have the centroid, we have to try to figure out
      // the cylinder's major axis.  Start by finding a point farthest
      // from the centroid.
      size_t i;
      double radius2 = 0.0;
      LPoint3d far_a = center;
      for (i = 0; i < num_vertices; i++) {
        double dist2 = (vpos[i] - center).length_squared();
        if (dist2 > radius2) {
          radius2 = dist2;
          far_a = vpos[i];
        }
      }

      // The point we have found above, far_a, must be one one of the
      // endcaps.  Now find another point, far_b, that is the farthest
      // from far_a.  This will be a point on the other endcap.
      radius2 = 0.0;
      LPoint3d far_b = center;
      for (i = 0; i < num_vertices; i++) {
        double dist2 = (vpos[i] - far_a).length_squared();
        if (dist2 > radius2) {
          radius2 = dist2;
          far_b = vpos[i];
        }
      }

      // Now we have far_a and far_b, one point on each endcap.
      // However, these points are not necessarily centered on the
      // endcaps, so we haven't figured out the cylinder's axis yet
      // (the line between far_a and far_b will probably pass through
      // the cylinder at an angle).

      // So we still need to determine the full set of points in each
      // endcap.  To do this, we pass back through the set of points,
      // categorizing each point into either "endcap a" or "endcap b".
      // We also leave a hefty chunk of points in the middle
      // uncategorized; this helps prevent us from getting a little
      // bit lopsided with points near the middle that may appear to
      // be closer to the wrong endcap.
      LPoint3d cap_a_center(0.0, 0.0, 0.0);
      LPoint3d cap_b_center(0.0, 0.0, 0.0);
      int num_a = 0;
      int num_b = 0;

      // This is the threshold length; points farther away from the
      // center than this are deemed to be in one endcap or the other.
      double center_length = (far_a - far_b).length() / 4.0;
      double center_length2 = center_length * center_length;

      for (i = 0; i < num_vertices; i++) {
        double dist2 = (vpos[i] - center).length_squared();
        if (dist2 > center_length2) {
          // This point is farther away from the center than
          // center_length; therefore it belongs in an endcap.
          double dist_a2 = (vpos[i] - far_a).length_squared();
          double dist_b2 = (vpos[i] - far_b).length_squared();
          if (dist_a2 < dist_b2) {
            // It's in endcap a.
            cap_a_center += vpos[i];
            num_a++;
          } else {
            // It's in endcap b.
            cap_b_center += vpos[i];
            num_b++;
          }
        }
      }

      if (num_a > 0 && num_b > 0) {
        cap_a_center /= (double)num_a;
        cap_b_center /= (double)num_b;


        // Now we finally have the major axis of the cylinder.
        LVector3d axis = cap_b_center - cap_a_center;
        axis.normalize();

        // If the axis is *almost* parallel with a major axis, assume
        // it is meant to be exactly parallel.
        if (IS_THRESHOLD_ZERO(axis[0], 0.01)) {
          axis[0] = 0.0;
        }
        if (IS_THRESHOLD_ZERO(axis[1], 0.01)) {
          axis[1] = 0.0;
        }
        if (IS_THRESHOLD_ZERO(axis[2], 0.01)) {
          axis[2] = 0.0;
        }
        axis.normalize();

        // Transform all of the points so that the major axis is along
        // the Y axis, and the origin is the center.  This is very
        // similar to the CollisionTube's idea of its canonical
        // orientation (although not exactly the same, since it is
        // centered on the origin instead of having point_a on the
        // origin).  It makes it easier to determine the length and
        // radius of the cylinder.
        LMatrix4d mat;
        look_at(mat, axis, LVector3d(0.0, 0.0, 1.0), CS_zup_right);
        mat.set_row(3, center);
        LMatrix4d inv_mat;
        inv_mat.invert_from(mat);

        for (i = 0; i < num_vertices; i++) {
          vpos[i] = vpos[i] * inv_mat;
        }

        double max_radius2 = 0.0;

        // Now determine the radius.
        for (i = 0; i < num_vertices; i++) {
          LVector2d v(vpos[i][0], vpos[i][2]);
          double radius2 = v.length_squared();
          if (radius2 > max_radius2) {
            max_radius2 = radius2;
          }
        }

        // And with the radius, we can determine the length.  We need
        // to know the radius first because we want the round endcaps
        // to enclose all points.
        double min_y = 0.0;
        double max_y = 0.0;

        for (i = 0; i < num_vertices; i++) {
          LVector2d v(vpos[i][0], vpos[i][2]);
          double radius2 = v.length_squared();

          if (vpos[i][1] < min_y) {
            // Adjust the Y pos to account for the point's distance
            // from the axis.
            double factor = sqrt(max_radius2 - radius2);
            min_y = min(min_y, vpos[i][1] + factor);

          } else if (vpos[i][1] > max_y) {
            double factor = sqrt(max_radius2 - radius2);
            max_y = max(max_y, vpos[i][1] - factor);
          }
        }

        double length = max_y - min_y;
        double radius = sqrt(max_radius2);

        // Finally, we have everything we need to define the cylinder.
        LVector3d half = axis * (length / 2.0);
        LPoint3d point_a = center - half;
        LPoint3d point_b = center + half;

        CollisionTube *cstube =
          new CollisionTube(LCAST(float, point_a), LCAST(float, point_b),
                            radius);
        apply_collision_flags(cstube, flags);
        cnode->add_solid(cstube);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::apply_collision_flags
//       Access: Private
//  Description: Does funny stuff to the CollisionSolid as
//               appropriate, based on the settings of the given
//               CollideFlags.
////////////////////////////////////////////////////////////////////
void EggLoader::
apply_collision_flags(CollisionSolid *solid,
                      EggGroup::CollideFlags flags) {
  if ((flags & EggGroup::CF_intangible) != 0) {
    solid->set_tangible(false);
  }
  if ((flags & EggGroup::CF_level) != 0) {
    solid->set_effective_normal(LVector3f::up());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::find_collision_geometry
//       Access: Private
//  Description: Looks for the node, at or below the indicated node,
//               that contains the associated collision geometry.
////////////////////////////////////////////////////////////////////
EggGroup *EggLoader::
find_collision_geometry(EggGroup *egg_group) {
  if ((egg_group->get_collide_flags() & EggGroup::CF_descend) != 0) {
    // If we have the "descend" instruction, we'll get to it when we
    // get to it.  Don't worry about it now.
    return egg_group;
  }

  // Does this group have any polygons?
  EggGroup::const_iterator ci;
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
      // Yes!  Use this group.
      return egg_group;
    }
  }

  // Well, the group had no polygons; look for a child group that has
  // the same collision type.
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggGroup::get_class_type())) {
      EggGroup *child_group = DCAST(EggGroup, *ci);
      if (child_group->get_cs_type() == egg_group->get_cs_type()) {
        return child_group;
      }
    }
  }

  // We got nothing.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::create_collision_plane
//       Access: Private
//  Description: Creates a single CollisionPlane from the indicated
//               EggPolygon.
////////////////////////////////////////////////////////////////////
CollisionPlane *EggLoader::
create_collision_plane(EggPolygon *egg_poly, EggGroup *parent_group) {
  if (!egg_poly->cleanup()) {
    egg2pg_cat.error()
      << "Degenerate collision plane in " << parent_group->get_name()
      << "\n";
    _error = true;
    return NULL;
  }

  LMatrix4d mat = egg_poly->get_vertex_to_node();

  pvector<Vertexf> vertices;
  if (!egg_poly->empty()) {
    EggPolygon::const_iterator vi;
    vi = egg_poly->begin();

    Vertexd vert = (*vi)->get_pos3() * mat;
    vertices.push_back(LCAST(float, vert));

    Vertexd last_vert = vert;
    ++vi;
    while (vi != egg_poly->end()) {
      vert = (*vi)->get_pos3() * mat;
      if (!vert.almost_equal(last_vert)) {
        vertices.push_back(LCAST(float, vert));
      }

      last_vert = vert;
      ++vi;
    }
  }

  if (vertices.size() < 3) {
    return NULL;
  }
  Planef plane(vertices[0], vertices[1], vertices[2]);
  return new CollisionPlane(plane);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::create_collision_polygons
//       Access: Private
//  Description: Creates one or more CollisionPolygons from the
//               indicated EggPolygon, and adds them to the indicated
//               CollisionNode.
////////////////////////////////////////////////////////////////////
void EggLoader::
create_collision_polygons(CollisionNode *cnode, EggPolygon *egg_poly,
                          EggGroup *parent_group,
                          EggGroup::CollideFlags flags) {
  LMatrix4d mat = egg_poly->get_vertex_to_node();

  PT(EggGroup) group = new EggGroup;

  if (!egg_poly->triangulate_into(group, false)) {
    egg2pg_cat.error()
      << "Degenerate collision polygon in " << parent_group->get_name()
      << "\n";
    _error = true;
    return;
  }

  if (group->size() != 1) {
    egg2pg_cat.error()
      << "Concave collision polygon in " << parent_group->get_name()
      << "\n";
    _error = true;
  }

  EggGroup::iterator ci;
  for (ci = group->begin(); ci != group->end(); ++ci) {
    EggPolygon *poly = DCAST(EggPolygon, *ci);

    pvector<Vertexf> vertices;
    if (!poly->empty()) {
      EggPolygon::const_iterator vi;
      vi = poly->begin();

      Vertexd vert = (*vi)->get_pos3() * mat;
      vertices.push_back(LCAST(float, vert));

      Vertexd last_vert = vert;
      ++vi;
      while (vi != poly->end()) {
        vert = (*vi)->get_pos3() * mat;
        if (!vert.almost_equal(last_vert)) {
          vertices.push_back(LCAST(float, vert));
        }

        last_vert = vert;
        ++vi;
      }
    }

    if (vertices.size() >= 3) {
      const Vertexf *vertices_begin = &vertices[0];
      const Vertexf *vertices_end = vertices_begin + vertices.size();
      PT(CollisionPolygon) cspoly =
        new CollisionPolygon(vertices_begin, vertices_end);
      if (cspoly->is_valid()) {
        apply_collision_flags(cspoly, flags);
        cnode->add_solid(cspoly);
      }        
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::apply_deferred_nodes
//       Access: Private
//  Description: Walks back over the tree and applies the
//               DeferredNodeProperties that were saved up along the
//               way.
////////////////////////////////////////////////////////////////////
void EggLoader::
apply_deferred_nodes(PandaNode *node, const DeferredNodeProperty &prop) {
  DeferredNodeProperty next_prop(prop);

  // Do we have a DeferredNodeProperty associated with this node?
  DeferredNodes::const_iterator dni;
  dni = _deferred_nodes.find(node);

  if (dni != _deferred_nodes.end()) {
    const DeferredNodeProperty &def = (*dni).second;
    next_prop.compose(def);
  }

  // Now apply the accumulated state to the node.
  next_prop.apply_to_node(node);

  int num_children = node->get_num_children();
  for (int i = 0; i < num_children; i++) {
    apply_deferred_nodes(node->get_child(i), next_prop);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::expand_object_types
//       Access: Private
//  Description: Recursively expands the group's ObjectType string(s).
//               It's recursive because an ObjectType string might
//               itself expand to another ObjectType string, which is
//               allowed; but we don't want to get caught in a cycle.
//
//               The return value is true if the object type is
//               expanded and the node is valid, or false if the node
//               should be ignored (e.g. ObjectType "backstage").
////////////////////////////////////////////////////////////////////
bool EggLoader::
expand_object_types(EggGroup *egg_group, const pset<string> &expanded,
                    const pvector<string> &expanded_history) {
  int num_object_types = egg_group->get_num_object_types();

  // First, copy out the object types so we can recursively modify the
  // list.
  vector_string object_types;
  int i;
  for (i = 0; i < num_object_types; i++) {
    object_types.push_back(egg_group->get_object_type(i));
  }
  egg_group->clear_object_types();

  for (i = 0; i < num_object_types; i++) {
    string object_type = object_types[i];
    pset<string> new_expanded(expanded);

    // Check for a cycle.
    if (!new_expanded.insert(object_type).second) {
      egg2pg_cat.error()
        << "Cycle in ObjectType expansions:\n";
      pvector<string>::const_iterator pi;
      for (pi = expanded_history.begin();
           pi != expanded_history.end();
           ++pi) {
        egg2pg_cat.error(false) 
          << (*pi) << " -> ";
      }
      egg2pg_cat.error(false) << object_type << "\n";
      _error = true;

    } else {
      // No cycle; continue.
      pvector<string> new_expanded_history(expanded_history);
      new_expanded_history.push_back(object_type);

      if (!do_expand_object_type(egg_group, new_expanded, 
                                 new_expanded_history, object_type)) {
        // Ignorable group; stop here.
        return false;
      }
      //else
        //egg2pg_cat.debug() << "returned true\n";

    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::expand_object_type
//       Access: Private
//  Description: Further implementation of expand_object_types().
////////////////////////////////////////////////////////////////////
bool EggLoader::
do_expand_object_type(EggGroup *egg_group, const pset<string> &expanded,
                      const pvector<string> &expanded_history,
                      const string &object_type) {
  // Try to find the egg syntax that the given objecttype is
  // shorthand for.  First, look in the config file.

  string egg_syntax =
    config_egg2pg.GetString("egg-object-type-" + downcase(object_type), "none");

  if (egg_syntax == "none") {
    // It wasn't defined in a config file.  Maybe it's built in?
    
    if (cmp_nocase_uh(object_type, "barrier") == 0) {
      egg_syntax = "<Collide> { Polyset descend }";
      
    } else if (cmp_nocase_uh(object_type, "solidpoly") == 0) {
      egg_syntax = "<Collide> { Polyset descend solid }";
      
    } else if (cmp_nocase_uh(object_type, "turnstile") == 0) {
      egg_syntax = "<Collide> { Polyset descend turnstile }";
      
    } else if (cmp_nocase_uh(object_type, "sphere") == 0) {
      egg_syntax = "<Collide> { Sphere descend }";

    } else if (cmp_nocase_uh(object_type, "tube") == 0) {
      egg_syntax = "<Collide> { Tube descend }";
      
    } else if (cmp_nocase_uh(object_type, "trigger") == 0) {
      egg_syntax = "<Collide> { Polyset descend intangible }";
      
    } else if (cmp_nocase_uh(object_type, "trigger_sphere") == 0) {
      egg_syntax = "<Collide> { Sphere descend intangible }";
      
    } else if (cmp_nocase_uh(object_type, "eye_trigger") == 0) {
      egg_syntax = "<Collide> { Polyset descend intangible center }";
      
    } else if (cmp_nocase_uh(object_type, "bubble") == 0) {
      egg_syntax = "<Collide> { Sphere keep descend }";
      
    } else if (cmp_nocase_uh(object_type, "ghost") == 0) {
      egg_syntax = "<Scalar> collide-mask { 0 }";
      
    } else if (cmp_nocase_uh(object_type, "dcs") == 0) {
      egg_syntax = "<DCS> { 1 }";
      
    } else if (cmp_nocase_uh(object_type, "model") == 0) {
      egg_syntax = "<Model> { 1 }";
      
    } else if (cmp_nocase_uh(object_type, "none") == 0) {
      // ObjectType "none" is a special case, meaning nothing in particular.
      return true;
      
    } else if (cmp_nocase_uh(object_type, "backstage") == 0) {
      // Ignore "backstage" geometry.
      return false;
      
    } else {
      egg2pg_cat.error()
        << "Unknown ObjectType " << object_type << "\n";
      _error = true;
      egg2pg_cat.debug() << "returning true\n";
      return true;
    }
  }

  if (!egg_syntax.empty()) {
    if (!egg_group->parse_egg(egg_syntax)) {
      egg2pg_cat.error()
        << "Error while parsing definition for ObjectType "
        << object_type << "\n";
      _error = true;

    } else {
      // Now we've parsed the object type syntax, which might have
      // added more object types.  Recurse if necessary.
      if (egg_group->get_num_object_types() != 0) {
        if (!expand_object_types(egg_group, expanded, expanded_history)) {
          return false;
        }
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_transform
//       Access: Private
//  Description: Walks back over the tree and applies the
//               DeferredNodeProperties that were saved up along the
//               way.
////////////////////////////////////////////////////////////////////
CPT(TransformState) EggLoader::
make_transform(const EggTransform3d *egg_transform) {
  // We'll build up the transform componentwise, so we preserve any
  // componentwise properties of the egg transform.

  CPT(TransformState) ts = TransformState::make_identity();
  int num_components = egg_transform->get_num_components();
  for (int i = 0; i < num_components; i++) {
    switch (egg_transform->get_component_type(i)) {
    case EggTransform3d::CT_translate:
      {
        LVector3f trans(LCAST(float, egg_transform->get_component_vector(i)));
        ts = TransformState::make_pos(trans)->compose(ts);
      }
      break;

    case EggTransform3d::CT_rotx:
      {
        LRotationf rot(LVector3f(1.0f, 0.0f, 0.0f),
                       (float)egg_transform->get_component_number(i));
        ts = TransformState::make_quat(rot)->compose(ts);
      }
      break;

    case EggTransform3d::CT_roty:
      {
        LRotationf rot(LVector3f(0.0f, 1.0f, 0.0f),
                       (float)egg_transform->get_component_number(i));
        ts = TransformState::make_quat(rot)->compose(ts);
      }
      break;

    case EggTransform3d::CT_rotz:
      {
        LRotationf rot(LVector3f(0.0f, 0.0f, 1.0f),
                       (float)egg_transform->get_component_number(i));
        ts = TransformState::make_quat(rot)->compose(ts);
      }
      break;

    case EggTransform3d::CT_rotate:
      {
        LRotationf rot(LCAST(float, egg_transform->get_component_vector(i)),
                       (float)egg_transform->get_component_number(i));
        ts = TransformState::make_quat(rot)->compose(ts);
      }
      break;

    case EggTransform3d::CT_scale:
      {
        LVecBase3f scale(LCAST(float, egg_transform->get_component_vector(i)));
        ts = TransformState::make_scale(scale)->compose(ts);
      }
      break;

    case EggTransform3d::CT_uniform_scale:
      {
        float scale = (float)egg_transform->get_component_number(i);
        ts = TransformState::make_scale(scale)->compose(ts);
      }
      break;

    case EggTransform3d::CT_matrix:
      {
        LMatrix4f mat(LCAST(float, egg_transform->get_component_matrix(i)));
        ts = TransformState::make_mat(mat)->compose(ts);
      }
      break;

    case EggTransform3d::CT_invalid:
      nassertr(false, ts);
      break;
    }
  }

  return ts;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::get_combine_mode
//       Access: Private, Static
//  Description: Extracts the combine_mode from the given egg texture,
//               and returns its corresponding TextureStage value.
////////////////////////////////////////////////////////////////////
TextureStage::CombineMode EggLoader::
get_combine_mode(const EggTexture *egg_tex, 
                 EggTexture::CombineChannel channel) {
  switch (egg_tex->get_combine_mode(channel)) {
  case EggTexture::CM_unspecified:
    // fall through

  case EggTexture::CM_modulate:
    return TextureStage::CM_modulate;

  case EggTexture::CM_replace:
    return TextureStage::CM_replace;

  case EggTexture::CM_add:
    return TextureStage::CM_add;

  case EggTexture::CM_add_signed:
    return TextureStage::CM_add_signed;

  case EggTexture::CM_interpolate:
    return TextureStage::CM_interpolate;

  case EggTexture::CM_subtract:
    return TextureStage::CM_subtract;

  case EggTexture::CM_dot3_rgb:
    return TextureStage::CM_dot3_rgb;

  case EggTexture::CM_dot3_rgba:
    return TextureStage::CM_dot3_rgba;
  };

  return TextureStage::CM_undefined;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::get_combine_source
//       Access: Private, Static
//  Description: Extracts the combine_source from the given egg texture,
//               and returns its corresponding TextureStage value.
////////////////////////////////////////////////////////////////////
TextureStage::CombineSource EggLoader::
get_combine_source(const EggTexture *egg_tex, 
                   EggTexture::CombineChannel channel, int n) {
  switch (egg_tex->get_combine_source(channel, n)) {
  case EggTexture::CS_unspecified:
    // The default source if it is unspecified is based on the
    // parameter index.
    switch (n) {
    case 0:
      return TextureStage::CS_previous;
    case 1:
      return TextureStage::CS_texture;
    case 2:
      return TextureStage::CS_constant;
    }
    // Otherwise, fall through

  case EggTexture::CS_texture:
    return TextureStage::CS_texture;

  case EggTexture::CS_constant:
    return TextureStage::CS_constant;

  case EggTexture::CS_primary_color:
    return TextureStage::CS_primary_color;

  case EggTexture::CS_previous:
    return TextureStage::CS_previous;
  };

  return TextureStage::CS_undefined;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::get_combine_operand
//       Access: Private, Static
//  Description: Extracts the combine_operand from the given egg texture,
//               and returns its corresponding TextureStage value.
////////////////////////////////////////////////////////////////////
TextureStage::CombineOperand EggLoader::
get_combine_operand(const EggTexture *egg_tex, 
                    EggTexture::CombineChannel channel, int n) {
  switch (egg_tex->get_combine_operand(channel, n)) {
  case EggTexture::CS_unspecified:
    if (channel == EggTexture::CC_rgb) {
      // The default operand for RGB is src_color, except for the
      // third parameter, which defaults to src_alpha.
      return n < 2 ? TextureStage::CO_src_color : TextureStage::CO_src_alpha;
    } else {
      // The default operand for alpha is always src_alpha.
      return TextureStage::CO_src_alpha;
    }

  case EggTexture::CO_src_color:
    return TextureStage::CO_src_color;

  case EggTexture::CO_one_minus_src_color:
    return TextureStage::CO_one_minus_src_color;

  case EggTexture::CO_src_alpha:
    return TextureStage::CO_src_alpha;

  case EggTexture::CO_one_minus_src_alpha:
    return TextureStage::CO_one_minus_src_alpha;
  };

  return TextureStage::CO_undefined;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::get_tex_gen
//       Access: Private, Static
//  Description: Extracts the tex_gen from the given egg texture,
//               and returns its corresponding TexGenAttrib mode.
////////////////////////////////////////////////////////////////////
TexGenAttrib::Mode EggLoader::
get_tex_gen(const EggTexture *egg_tex) {
  switch (egg_tex->get_tex_gen()) {
  case EggTexture::TG_unspecified:
    return TexGenAttrib::M_off;

  case EggTexture::TG_sphere_map:
    return TexGenAttrib::M_sphere_map;

  case EggTexture::TG_cube_map:
    return TexGenAttrib::M_cube_map;

  case EggTexture::TG_world_position:
    return TexGenAttrib::M_world_position;

  case EggTexture::TG_object_position:
    return TexGenAttrib::M_object_position;

  case EggTexture::TG_eye_position:
    return TexGenAttrib::M_eye_position;
  };

  return TexGenAttrib::M_off;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::apply_tex_mat
//       Access: Private, Static
//  Description: Applies the texture matrix from the indicated egg
//               texture to the given TexMatrixAttrib, and returns the
//               new attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) EggLoader::
apply_tex_mat(CPT(RenderAttrib) tex_mat_attrib, 
              TextureStage *stage, const EggTexture *egg_tex) {
  if (egg_tex->has_transform()) {
    const LMatrix3d &tex_mat = egg_tex->get_transform();
    LMatrix4f mat4(tex_mat(0, 0), tex_mat(0, 1), 0.0f, tex_mat(0, 2),
                   tex_mat(1, 0), tex_mat(1, 1), 0.0f, tex_mat(1, 2),
                   0.0f, 0.0f, 1.0f, 0.0f,
                   tex_mat(2, 0), tex_mat(2, 1), 0.0f, tex_mat(2, 2));
    CPT(TransformState) transform = TransformState::make_mat(mat4);
  
    if (tex_mat_attrib == (const RenderAttrib *)NULL) {
      tex_mat_attrib = TexMatrixAttrib::make();
    }
    tex_mat_attrib = DCAST(TexMatrixAttrib, tex_mat_attrib)->
      add_stage(stage, transform);
  }
    
  return tex_mat_attrib;
}

  
