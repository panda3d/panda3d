/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggLoader.cxx
 * @author drose
 * @date 2002-02-26
 */

#include "pandabase.h"

#include "eggLoader.h"
#include "eggRenderState.h"
#include "egg_parametrics.h"
#include "config_egg2pg.h"
#include "config_egg.h"
#include "nodePath.h"
#include "renderState.h"
#include "transformState.h"
#include "texturePool.h"
#include "billboardEffect.h"
#include "decalEffect.h"
#include "colorAttrib.h"
#include "textureAttrib.h"
#include "materialPool.h"
#include "geomNode.h"
#include "geomVertexFormat.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geom.h"
#include "geomTriangles.h"
#include "geomTristrips.h"
#include "geomTrifans.h"
#include "geomLines.h"
#include "geomLinestrips.h"
#include "geomPoints.h"
#include "geomPatches.h"
#include "sequenceNode.h"
#include "switchNode.h"
#include "portalNode.h"
#include "occluderNode.h"
#include "polylightNode.h"
#include "lodNode.h"
#include "modelNode.h"
#include "modelRoot.h"
#include "string_utils.h"
#include "eggPrimitive.h"
#include "eggPatch.h"
#include "eggPoint.h"
#include "eggLine.h"
#include "eggTextureCollection.h"
#include "eggNurbsCurve.h"
#include "eggNurbsSurface.h"
#include "eggGroupNode.h"
#include "eggGroup.h"
#include "eggPolygon.h"
#include "eggTriangleStrip.h"
#include "eggTriangleFan.h"
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
#include "collisionInvSphere.h"
#include "collisionCapsule.h"
#include "collisionPlane.h"
#include "collisionPolygon.h"
#include "collisionFloorMesh.h"
#include "collisionBox.h"
#include "parametricCurve.h"
#include "nurbsCurve.h"
#include "nurbsCurveInterface.h"
#include "nurbsCurveEvaluator.h"
#include "nurbsSurfaceEvaluator.h"
#include "ropeNode.h"
#include "sheetNode.h"
#include "look_at.h"
#include "configVariableString.h"
#include "transformBlendTable.h"
#include "transformBlend.h"
#include "sparseArray.h"
#include "bitArray.h"
#include "thread.h"
#include "uvScrollNode.h"
#include "textureStagePool.h"
#include "cmath.h"

#include <ctype.h>
#include <algorithm>

using std::max;
using std::min;
using std::string;

// This class is used in make_node(EggBin *) to sort LOD instances in order by
// switching distance.
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
  nassertv(egg_node != nullptr);
  _egg_node = egg_node;

  // We expect this egg node to be an EggGroup with an LOD specification.
  // That's what the EggBinner collected together, after all.
  EggGroup *egg_group = DCAST(EggGroup, egg_node);
  nassertv(egg_group->has_lod());
  const EggSwitchCondition &sw = egg_group->get_lod();

  // For now, this is the only kind of switch condition there is.
  _d = DCAST(EggSwitchConditionDistance, &sw);
}


/**
 *
 */
EggLoader::
EggLoader() {
  // We need to enforce whatever coordinate system the user asked for.
  _data = new EggData;
  _data->set_coordinate_system(egg_coordinate_system);
  _error = false;
  _dynamic_override = false;
  _dynamic_override_char_maker = nullptr;
}

/**
 * The EggLoader constructor makes a copy of the EggData passed in.
 */
EggLoader::
EggLoader(const EggData *data) :
  _data(new EggData(*data))
{
  _error = false;
  _dynamic_override = false;
  _dynamic_override_char_maker = nullptr;
}


/**
 *
 */
void EggLoader::
build_graph() {
  _deferred_nodes.clear();

  // Expand all of the ObjectType flags before we do anything else; that might
  // prune out large portions of the scene.
  if (!expand_all_object_types(_data)) {
    return;
  }

  // Now, load up all of the textures.
  load_textures();

  // Clean up the vertices.
  _data->clear_connected_shading();
  _data->remove_unused_vertices(true);
  _data->get_connected_shading();
  _data->unify_attributes(true, egg_flat_shading, true);

  // Now we need to get the connected shading again, since in unifying the
  // attributes we may have made vertices suddenly become identical to each
  // other, thereby connecting more primitives than before.
  _data->clear_connected_shading();
  _data->remove_unused_vertices(true);
  _data->get_connected_shading();

  // Sequences and switches have special needs.  Make sure that primitives
  // parented directly to a sequence or switch are sorted into sub-groups
  // first, to prevent them being unified into a single polyset.
  separate_switches(_data);

  if (egg_emulate_bface) {
    emulate_bface(_data);
  }

  // Then bin up the polysets and LOD nodes.
  _data->remove_invalid_primitives(true);
  EggBinner binner(*this);
  binner.make_bins(_data);

  // ((EggGroupNode *)_data)->write(cerr, 0);

  // Now build up the scene graph.
  _root = new ModelRoot(_data->get_egg_filename(), _data->get_egg_timestamp());

  EggGroupNode::const_iterator ci;
  for (ci = _data->begin(); ci != _data->end(); ++ci) {
    make_node(*ci, _root);
  }

  reparent_decals();
  start_sequences();

  apply_deferred_nodes(_root, DeferredNodeProperty());
}

/**
 * For each node representing a decal base geometry (i.e.  a node
 * corresponding to an EggGroup with the decal flag set), move all of its
 * nested geometry directly below the GeomNode representing the group.
 */
void EggLoader::
reparent_decals() {
  ExtraNodes::const_iterator di;
  for (di = _decals.begin(); di != _decals.end(); ++di) {
    PandaNode *node = (*di);
    nassertv(node != nullptr);

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
      // Now reparent all of the non-GeomNodes to this node.  We have to be
      // careful so we don't get lost as we self-modify this list.
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

/**
 * Starts all of the SequenceNodes we created looping.  We have to wait until
 * the entire graph is built up to do this, because the SequenceNode needs its
 * full set of children before it can know how many frames to loop.
 */
void EggLoader::
start_sequences() {
  ExtraNodes::const_iterator ni;
  for (ni = _sequences.begin(); ni != _sequences.end(); ++ni) {
    SequenceNode *node = DCAST(SequenceNode, (*ni));
    node->loop(true);
  }
}

/**
 * Creates a polyset--that is, a Geom--from the primitives that have already
 * been grouped into a bin.  If transform is non-NULL, it represents the
 * transform to apply to the vertices (instead of the default transform based
 * on the bin's position within the hierarchy).
 */
void EggLoader::
make_polyset(EggBin *egg_bin, PandaNode *parent, const LMatrix4d *transform,
             bool is_dynamic, CharacterMaker *character_maker) {
  if (egg_bin->empty()) {
    // If there are no children--no primitives--never mind.
    return;
  }

  // We know that all of the primitives in the bin have the same render state,
  // so we can get that information from the first primitive.
  EggGroupNode::const_iterator ci = egg_bin->begin();
  nassertv(ci != egg_bin->end());
  CPT(EggPrimitive) first_prim = DCAST(EggPrimitive, (*ci));
  nassertv(first_prim != nullptr);
  const EggRenderState *render_state;
  DCAST_INTO_V(render_state, first_prim->get_user_data(EggRenderState::get_class_type()));

  if (render_state->_hidden && egg_suppress_hidden) {
    // Eat this polyset.
    return;
  }

  // Generate an optimal vertex pool (or multiple vertex pools, if we have a
  // lot of vertex) for the polygons within just the bin.  Each EggVertexPool
  // translates directly to an optimal GeomVertexData structure.
  EggVertexPools vertex_pools;
  egg_bin->rebuild_vertex_pools(vertex_pools, (unsigned int)egg_max_vertices,
                                false);

  if (egg_mesh) {
    // If we're using the mesher, mesh now.
    egg_bin->mesh_triangles(render_state->_flat_shaded ? EggGroupNode::T_flat_shaded : 0);

  } else {
    // If we're not using the mesher, at least triangulate any higher-order
    // polygons we might have.
    egg_bin->triangulate_polygons(EggGroupNode::T_polygon | EggGroupNode::T_convex);
  }

  // Now that we've meshed, apply the per-prim attributes onto the vertices,
  // so we can copy them to the GeomVertexData.
  egg_bin->apply_first_attribute(false);
  egg_bin->post_apply_flat_attribute(false);

  // egg_bin->write(cerr, 0);

  PT(GeomNode) geom_node;

  // Now iterate through each EggVertexPool.  Normally, there's only one, but
  // if we have a really big mesh, it might have been split into multiple
  // vertex pools (to keep each one within the egg_max_vertices constraint).
  EggVertexPools::iterator vpi;
  for (vpi = vertex_pools.begin(); vpi != vertex_pools.end(); ++vpi) {
    EggVertexPool *vertex_pool = (*vpi);
    vertex_pool->remove_unused_vertices();
    // vertex_pool->write(cerr, 0);

    bool has_overall_color;
    LColor overall_color;
    vertex_pool->check_overall_color(has_overall_color, overall_color);
    if (!egg_flat_colors) {
      // If flat colors aren't allowed, then we don't care whether there is an
      // overall color.  In that case, treat all vertex pools as if they
      // contain a combination of multiple colors.
      has_overall_color = false;
    }

    PT(TransformBlendTable) blend_table;
    if (is_dynamic) {
      // Dynamic vertex pools will require a TransformBlendTable to indicate
      // how the vertices are to be animated.
      blend_table = make_blend_table(vertex_pool, egg_bin, character_maker);

      // Now that we've created the blend table, we can re-order the vertices
      // in the pool to efficiently group vertices together that will share
      // the same transform matrix.  (We have to re-order them before we
      // create primitives, below, because this will change the vertex index
      // numbers.)
      vertex_pool->sort_by_external_index();
    }

    // Create a handful of GeomPrimitives corresponding to the various types
    // of primitives that reference this vertex pool.
    UniquePrimitives unique_primitives;
    Primitives primitives;
    for (ci = egg_bin->begin(); ci != egg_bin->end(); ++ci) {
      EggPrimitive *egg_prim;
      DCAST_INTO_V(egg_prim, (*ci));
      if (egg_prim->get_pool() == vertex_pool) {
        make_primitive(render_state, egg_prim, unique_primitives, primitives,
                       has_overall_color, overall_color);
      }
    }

    if (!primitives.empty()) {
      LMatrix4d mat;
      if (transform != nullptr) {
        mat = (*transform);
      } else {
        mat = egg_bin->get_vertex_to_node();
      }

      // Now convert this vertex pool to a GeomVertexData.
      PT(GeomVertexData) vertex_data =
        make_vertex_data(render_state, vertex_pool, egg_bin, mat, blend_table,
                         is_dynamic, character_maker, has_overall_color);
      nassertv(vertex_data != nullptr);

      // And create a Geom to hold the primitives.
      PT(Geom) geom = new Geom(vertex_data);

      // Add each new primitive to the Geom.
      Primitives::const_iterator pi;
      for (pi = primitives.begin(); pi != primitives.end(); ++pi) {
        PT(GeomPrimitive) primitive = (*pi);

        if (primitive->is_indexed()) {
          // Since we may have over-allocated while we were filling up the
          // primitives, down-allocate now.
          primitive->reserve_num_vertices(primitive->get_num_vertices());
        }

        geom->add_primitive(primitive);
      }

        // vertex_data->write(cerr); geom->write(cerr);
        // render_state->_state->write(cerr, 0);

      // Create a new GeomNode if we haven't already.
      if (geom_node == nullptr) {
        // Now, is our parent node a GeomNode, or just an ordinary PandaNode?
        // If it's a GeomNode, we can add the new Geom directly to our parent;
        // otherwise, we need to create a new node.
        if (parent->is_geom_node() && !render_state->_hidden) {
          geom_node = DCAST(GeomNode, parent);

        } else {
          geom_node = new GeomNode(egg_bin->get_name());
          if (render_state->_hidden) {
            parent->add_stashed(geom_node);
          } else {
            parent->add_child(geom_node);
          }
        }
      }

      CPT(RenderState) geom_state = render_state->_state;
      if (has_overall_color) {
        if (!overall_color.almost_equal(LColor(1.0f, 1.0f, 1.0f, 1.0f))) {
          geom_state = geom_state->add_attrib(ColorAttrib::make_flat(overall_color), -1);
        }
      } else {
        geom_state = geom_state->add_attrib(ColorAttrib::make_vertex(), -1);
      }

      geom_node->add_geom(geom, geom_state);
    }
  }

  if (geom_node != nullptr && egg_show_normals) {
    // Create some more geometry to visualize each normal.
    for (vpi = vertex_pools.begin(); vpi != vertex_pools.end(); ++vpi) {
      EggVertexPool *vertex_pool = (*vpi);
      show_normals(vertex_pool, geom_node);
    }
  }
}

/**
 * Creates a TransformState object corresponding to the indicated
 * EggTransform.
 */
CPT(TransformState) EggLoader::
make_transform(const EggTransform *egg_transform) {
  // We'll build up the transform componentwise, so we preserve any
  // componentwise properties of the egg transform.

  CPT(TransformState) ts = TransformState::make_identity();
  int num_components = egg_transform->get_num_components();
  for (int i = 0; i < num_components; i++) {
    switch (egg_transform->get_component_type(i)) {
    case EggTransform::CT_translate2d:
      {
        LVecBase2 trans2d(LCAST(PN_stdfloat, egg_transform->get_component_vec2(i)));
        LVecBase3 trans3d(trans2d[0], trans2d[1], 0.0f);
        ts = TransformState::make_pos(trans3d)->compose(ts);
      }
      break;

    case EggTransform::CT_translate3d:
      {
        LVecBase3 trans3d(LCAST(PN_stdfloat, egg_transform->get_component_vec3(i)));
        ts = TransformState::make_pos(trans3d)->compose(ts);
      }
      break;

    case EggTransform::CT_rotate2d:
      {
        LRotation rot(LVector3(0.0f, 0.0f, 1.0f),
                       (PN_stdfloat)egg_transform->get_component_number(i));
        ts = TransformState::make_quat(rot)->compose(ts);
      }
      break;

    case EggTransform::CT_rotx:
      {
        LRotation rot(LVector3(1.0f, 0.0f, 0.0f),
                       (PN_stdfloat)egg_transform->get_component_number(i));
        ts = TransformState::make_quat(rot)->compose(ts);
      }
      break;

    case EggTransform::CT_roty:
      {
        LRotation rot(LVector3(0.0f, 1.0f, 0.0f),
                       (PN_stdfloat)egg_transform->get_component_number(i));
        ts = TransformState::make_quat(rot)->compose(ts);
      }
      break;

    case EggTransform::CT_rotz:
      {
        LRotation rot(LVector3(0.0f, 0.0f, 1.0f),
                       (PN_stdfloat)egg_transform->get_component_number(i));
        ts = TransformState::make_quat(rot)->compose(ts);
      }
      break;

    case EggTransform::CT_rotate3d:
      {
        LRotation rot(LCAST(PN_stdfloat, egg_transform->get_component_vec3(i)),
                       (PN_stdfloat)egg_transform->get_component_number(i));
        ts = TransformState::make_quat(rot)->compose(ts);
      }
      break;

    case EggTransform::CT_scale2d:
      {
        LVecBase2 scale2d(LCAST(PN_stdfloat, egg_transform->get_component_vec2(i)));
        LVecBase3 scale3d(scale2d[0], scale2d[1], 1.0f);
        ts = TransformState::make_scale(scale3d)->compose(ts);
      }
      break;

    case EggTransform::CT_scale3d:
      {
        LVecBase3 scale3d(LCAST(PN_stdfloat, egg_transform->get_component_vec3(i)));
        ts = TransformState::make_scale(scale3d)->compose(ts);
      }
      break;

    case EggTransform::CT_uniform_scale:
      {
        PN_stdfloat scale = (PN_stdfloat)egg_transform->get_component_number(i);
        ts = TransformState::make_scale(scale)->compose(ts);
      }
      break;

    case EggTransform::CT_matrix3:
      {
        LMatrix3 m(LCAST(PN_stdfloat, egg_transform->get_component_mat3(i)));
        LMatrix4 mat4(m(0, 0), m(0, 1), 0.0, m(0, 2),
                       m(1, 0), m(1, 1), 0.0, m(1, 2),
                       0.0, 0.0, 1.0, 0.0,
                       m(2, 0), m(2, 1), 0.0, m(2, 2));

        ts = TransformState::make_mat(mat4)->compose(ts);
      }
      break;

    case EggTransform::CT_matrix4:
      {
        LMatrix4 mat4(LCAST(PN_stdfloat, egg_transform->get_component_mat4(i)));
        ts = TransformState::make_mat(mat4)->compose(ts);
      }
      break;

    case EggTransform::CT_invalid:
      nassertr(false, ts);
      break;
    }
  }

  if (ts->components_given()) {
    return ts;
  }

  // Finally, we uniquify all the matrix-based TransformStates we create by
  // complete matrix value.  The TransformState class doesn't normally go this
  // far, because of the cost of this additional uniquification step, but this
  // is the egg loader so we don't mind spending a little bit of extra time
  // here to get a more optimal result.
  TransformStates::iterator tsi = _transform_states.insert(TransformStates::value_type(ts->get_mat(), ts)).first;

  return (*tsi).second;
}

/**
 * In the presence of egg-show-normals, generate some additional geometry to
 * represent the normals, tangents, and binormals of each vertex.
 */
void EggLoader::
show_normals(EggVertexPool *vertex_pool, GeomNode *geom_node) {
  PT(GeomPrimitive) primitive = new GeomLines(Geom::UH_static);
  CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3cp();
  PT(GeomVertexData) vertex_data =
    new GeomVertexData(vertex_pool->get_name(), format, Geom::UH_static);

  GeomVertexWriter vertex(vertex_data, InternalName::get_vertex());
  GeomVertexWriter color(vertex_data, InternalName::get_color());

  EggVertexPool::const_iterator vi;
  for (vi = vertex_pool->begin(); vi != vertex_pool->end(); ++vi) {
    EggVertex *vert = (*vi);
    LPoint3d pos = vert->get_pos3();

    if (vert->has_normal()) {
      vertex.add_data3d(pos);
      vertex.add_data3d(pos + vert->get_normal() * egg_normal_scale);
      color.add_data4(1.0f, 0.0f, 0.0f, 1.0f);
      color.add_data4(1.0f, 0.0f, 0.0f, 1.0f);
      primitive->add_next_vertices(2);
      primitive->close_primitive();
    }

    // Also look for tangents and binormals in each texture coordinate set.
    EggVertex::const_uv_iterator uvi;
    for (uvi = vert->uv_begin(); uvi != vert->uv_end(); ++uvi) {
      EggVertexUV *uv_obj = (*uvi);
      if (uv_obj->has_tangent()) {
        vertex.add_data3d(pos);
        vertex.add_data3d(pos + uv_obj->get_tangent() * egg_normal_scale);
        color.add_data4(0.0f, 1.0f, 0.0f, 1.0f);
        color.add_data4(0.0f, 1.0f, 0.0f, 1.0f);
        primitive->add_next_vertices(2);
        primitive->close_primitive();
      }
      if (uv_obj->has_binormal()) {
        vertex.add_data3d(pos);
        vertex.add_data3d(pos + uv_obj->get_binormal() * egg_normal_scale);
        color.add_data4(0.0f, 0.0f, 1.0f, 1.0f);
        color.add_data4(0.0f, 0.0f, 1.0f, 1.0f);
        primitive->add_next_vertices(2);
        primitive->close_primitive();
      }
    }
  }

  PT(Geom) geom = new Geom(vertex_data);
  geom->add_primitive(primitive);
  geom_node->add_geom(geom);
}

/**
 *
 */
void EggLoader::
make_nurbs_curve(EggNurbsCurve *egg_curve, PandaNode *parent,
                 const LMatrix4d &mat) {
  if (egg_load_old_curves) {
    // Make a NurbsCurve instead of a RopeNode (old interface).
    make_old_nurbs_curve(egg_curve, parent, mat);
    return;
  }

  assert(parent != nullptr);
  assert(!parent->is_geom_node());

  PT(NurbsCurveEvaluator) nurbs = ::make_nurbs_curve(egg_curve, mat);
  if (nurbs == nullptr) {
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

  const EggRenderState *render_state;
  DCAST_INTO_V(render_state, egg_curve->get_user_data(EggRenderState::get_class_type()));
  if (render_state->_hidden && egg_suppress_hidden) {
    // Eat this primitive.
    return;
  }

  rope->set_state(render_state->_state);
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

/**
 * This deprecated interface creates a NurbsCurve object for the EggNurbsCurve
 * entry.  It will eventually be removed in favor of the above, which creates
 * a RopeNode.
 *
 * @deprecated See make_nurbs_curve.
 */
void EggLoader::
make_old_nurbs_curve(EggNurbsCurve *egg_curve, PandaNode *parent,
                     const LMatrix4d &mat) {
  assert(parent != nullptr);
  assert(!parent->is_geom_node());

  PT(ParametricCurve) curve;
  curve = new NurbsCurve;

  NurbsCurveInterface *nurbs = curve->get_nurbs_interface();
  nassertv(nurbs != nullptr);

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
    nurbs->append_cv(LCAST(PN_stdfloat, (*pi)->get_pos4() * mat));
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

/**
 *
 */
void EggLoader::
make_nurbs_surface(EggNurbsSurface *egg_surface, PandaNode *parent,
                   const LMatrix4d &mat) {
  assert(parent != nullptr);
  assert(!parent->is_geom_node());

  PT(NurbsSurfaceEvaluator) nurbs = ::make_nurbs_surface(egg_surface, mat);
  if (nurbs == nullptr) {
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

  const EggRenderState *render_state;
  DCAST_INTO_V(render_state, egg_surface->get_user_data(EggRenderState::get_class_type()));
  if (render_state->_hidden && egg_suppress_hidden) {
    // Eat this primitive.
    return;
  }

  sheet->set_state(render_state->_state);

  if (egg_surface->has_vertex_color()) {
    // If the surface had individual vertex color, enable it.
    sheet->set_use_vertex_color(true);
  } else if (egg_surface->has_color()) {
    // Otherwise, if the surface has overall color, apply it.
    sheet->set_attrib(ColorAttrib::make_flat(egg_surface->get_color()));
  }

  parent->add_child(sheet);
}

/**
 *
 */
void EggLoader::
load_textures() {
  // First, collect all the textures that are referenced.
  EggTextureCollection tc;
  tc.find_used_textures(_data);

  EggTextureCollection::iterator ti;
  for (ti = tc.begin(); ti != tc.end(); ++ti) {
    PT_EggTexture egg_tex = (*ti);

    TextureDef def;
    if (load_texture(def, egg_tex)) {
      // Now associate the pointers, so we'll be able to look up the Texture
      // pointer given an EggTexture pointer, later.
      _textures[egg_tex] = def;
    }
  }
}


/**
 *
 */
bool EggLoader::
load_texture(TextureDef &def, EggTexture *egg_tex) {
  // Check to see if we should reduce the number of channels in the texture.
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
    wanted_alpha = egg_tex->has_alpha_filename();
  }

  // Since some properties of the textures are inferred from the texture files
  // themselves (if the properties are not explicitly specified in the egg
  // file), then we add the textures as dependents for the egg file.
  if (_record != nullptr) {
    _record->add_dependent_file(egg_tex->get_fullpath());
    if (egg_tex->has_alpha_filename() && wanted_alpha) {
      _record->add_dependent_file(egg_tex->get_alpha_fullpath());
    }
  }

  // By convention, the egg loader will preload the simple texture images.
  LoaderOptions options;
  if (egg_preload_simple_textures) {
    options.set_texture_flags(options.get_texture_flags() | LoaderOptions::TF_preload_simple);
  }

  if (!egg_ignore_filters && !egg_ignore_mipmaps) {
    switch (egg_tex->get_minfilter()) {
    case EggTexture::FT_nearest:
    case EggTexture::FT_linear:
    case EggTexture::FT_unspecified:
      break;

    case EggTexture::FT_nearest_mipmap_nearest:
    case EggTexture::FT_linear_mipmap_nearest:
    case EggTexture::FT_nearest_mipmap_linear:
    case EggTexture::FT_linear_mipmap_linear:
      options.set_texture_flags(options.get_texture_flags() | LoaderOptions::TF_generate_mipmaps);
    }
  }

  if (egg_tex->get_multiview()) {
    options.set_texture_flags(options.get_texture_flags() | LoaderOptions::TF_multiview);
    if (egg_tex->has_num_views()) {
      options.set_texture_num_views(egg_tex->get_num_views());
    }
  }

  // Allow the texture loader to pre-compress the texture.
  if (egg_tex->get_compression_mode() == EggTexture::CM_on) {
    options.set_texture_flags(options.get_texture_flags() | LoaderOptions::TF_allow_compression);
  }

  PT(Texture) tex;
  switch (egg_tex->get_texture_type()) {
  case EggTexture::TT_unspecified:
  case EggTexture::TT_1d_texture:
    options.set_texture_flags(options.get_texture_flags() | LoaderOptions::TF_allow_1d);
    // Fall through.

  case EggTexture::TT_2d_texture:
    if (egg_tex->has_alpha_filename() && wanted_alpha) {
      tex = TexturePool::load_texture(egg_tex->get_fullpath(),
                                      egg_tex->get_alpha_fullpath(),
                                      wanted_channels,
                                      egg_tex->get_alpha_file_channel(),
                                      egg_tex->get_read_mipmaps(), options);
    } else {
      tex = TexturePool::load_texture(egg_tex->get_fullpath(),
                                      wanted_channels,
                                      egg_tex->get_read_mipmaps(), options);
    }
    break;

  case EggTexture::TT_3d_texture:
    tex = TexturePool::load_3d_texture(egg_tex->get_fullpath(),
                                       egg_tex->get_read_mipmaps(), options);
    break;

  case EggTexture::TT_cube_map:
    tex = TexturePool::load_cube_map(egg_tex->get_fullpath(),
                                     egg_tex->get_read_mipmaps(), options);
    break;
  }

  if (tex == nullptr) {
    return false;
  }

  // Record the original filenames in the textures (as loaded from the egg
  // file).  These filenames will be written back to the bam file if the bam
  // file is written out.
  tex->set_filename(egg_tex->get_filename());
  if (egg_tex->has_alpha_filename() && wanted_alpha) {
    tex->set_alpha_filename(egg_tex->get_alpha_filename());
  }

  // See if there is some egg data hanging on the texture.  In particular, the
  // TxaFileFilter might have left that here for us.
  TypedReferenceCount *aux = tex->get_aux_data("egg");
  if (aux != nullptr &&
      aux->is_of_type(EggTexture::get_class_type())) {
    EggTexture *aux_egg_tex = DCAST(EggTexture, aux);

    if (aux_egg_tex->get_alpha_mode() != EggTexture::AM_unspecified) {
      egg_tex->set_alpha_mode(aux_egg_tex->get_alpha_mode());
    }
    if (aux_egg_tex->get_format() != EggTexture::F_unspecified) {
      egg_tex->set_format(aux_egg_tex->get_format());
    }
    if (aux_egg_tex->get_minfilter() != EggTexture::FT_unspecified) {
      egg_tex->set_minfilter(aux_egg_tex->get_minfilter());
    }
    if (aux_egg_tex->get_magfilter() != EggTexture::FT_unspecified) {
      egg_tex->set_magfilter(aux_egg_tex->get_magfilter());
    }
    if (aux_egg_tex->has_anisotropic_degree()) {
      egg_tex->set_anisotropic_degree(aux_egg_tex->get_anisotropic_degree());
    }
  }

  apply_texture_attributes(tex, egg_tex);

  // Make a texture stage for the texture.
  PT(TextureStage) stage = make_texture_stage(egg_tex);
  def._texture = DCAST(TextureAttrib, TextureAttrib::make())->add_on_stage(stage, tex);
  def._stage = stage;
  def._egg_tex = egg_tex;

  return true;
}


/**
 *
 */
void EggLoader::
apply_texture_attributes(Texture *tex, const EggTexture *egg_tex) {
  if (egg_tex->get_compression_mode() != EggTexture::CM_default) {
    tex->set_compression(convert_compression_mode(egg_tex->get_compression_mode()));
  }

  SamplerState sampler;

  EggTexture::WrapMode wrap_u = egg_tex->determine_wrap_u();
  EggTexture::WrapMode wrap_v = egg_tex->determine_wrap_v();
  EggTexture::WrapMode wrap_w = egg_tex->determine_wrap_w();

  if (wrap_u != EggTexture::WM_unspecified) {
    sampler.set_wrap_u(convert_wrap_mode(wrap_u));
  }
  if (wrap_v != EggTexture::WM_unspecified) {
    sampler.set_wrap_v(convert_wrap_mode(wrap_v));
  }
  if (wrap_w != EggTexture::WM_unspecified) {
    sampler.set_wrap_w(convert_wrap_mode(wrap_w));
  }

  if (egg_tex->has_border_color()) {
    sampler.set_border_color(egg_tex->get_border_color());
  }

  switch (egg_tex->get_minfilter()) {
  case EggTexture::FT_nearest:
    sampler.set_minfilter(SamplerState::FT_nearest);
    break;

  case EggTexture::FT_linear:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      sampler.set_minfilter(SamplerState::FT_nearest);
    } else {
      sampler.set_minfilter(SamplerState::FT_linear);
    }
    break;

  case EggTexture::FT_nearest_mipmap_nearest:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      sampler.set_minfilter(SamplerState::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2pg_cat.warning()
        << "Ignoring mipmap request\n";
      sampler.set_minfilter(SamplerState::FT_nearest);
    } else {
      sampler.set_minfilter(SamplerState::FT_nearest_mipmap_nearest);
    }
    break;

  case EggTexture::FT_linear_mipmap_nearest:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      sampler.set_minfilter(SamplerState::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2pg_cat.warning()
        << "Ignoring mipmap request\n";
      sampler.set_minfilter(SamplerState::FT_linear);
    } else {
      sampler.set_minfilter(SamplerState::FT_linear_mipmap_nearest);
    }
    break;

  case EggTexture::FT_nearest_mipmap_linear:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      sampler.set_minfilter(SamplerState::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2pg_cat.warning()
        << "Ignoring mipmap request\n";
      sampler.set_minfilter(SamplerState::FT_nearest);
    } else {
      sampler.set_minfilter(SamplerState::FT_nearest_mipmap_linear);
    }
    break;

  case EggTexture::FT_linear_mipmap_linear:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring minfilter request\n";
      sampler.set_minfilter(SamplerState::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2pg_cat.warning()
        << "Ignoring mipmap request\n";
      sampler.set_minfilter(SamplerState::FT_linear);
    } else {
      sampler.set_minfilter(SamplerState::FT_linear_mipmap_linear);
    }
    break;

  case EggTexture::FT_unspecified:
    break;
  }

  switch (egg_tex->get_magfilter()) {
  case EggTexture::FT_nearest:
  case EggTexture::FT_nearest_mipmap_nearest:
  case EggTexture::FT_nearest_mipmap_linear:
    sampler.set_magfilter(SamplerState::FT_nearest);
    break;

  case EggTexture::FT_linear:
  case EggTexture::FT_linear_mipmap_nearest:
  case EggTexture::FT_linear_mipmap_linear:
    if (egg_ignore_filters) {
      egg2pg_cat.warning()
        << "Ignoring magfilter request\n";
      sampler.set_magfilter(SamplerState::FT_nearest);
    } else {
      sampler.set_magfilter(SamplerState::FT_linear);
    }
    break;

  case EggTexture::FT_unspecified:
    break;
  }

  if (egg_tex->has_anisotropic_degree()) {
    sampler.set_anisotropic_degree(egg_tex->get_anisotropic_degree());
  }

  if (egg_tex->has_min_lod()) {
    sampler.set_min_lod(egg_tex->get_min_lod());
  }

  if (egg_tex->has_max_lod()) {
    sampler.set_max_lod(egg_tex->get_max_lod());
  }

  if (egg_tex->has_lod_bias()) {
    sampler.set_lod_bias(egg_tex->get_lod_bias());
  }

  tex->set_default_sampler(sampler);

  if (tex->get_num_components() == 1) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_red:
      tex->set_format(Texture::F_red);
      break;
    case EggTexture::F_green:
      tex->set_format(Texture::F_green);
      break;
    case EggTexture::F_blue:
      tex->set_format(Texture::F_blue);
      break;
    case EggTexture::F_alpha:
      tex->set_format(Texture::F_alpha);
      break;
    case EggTexture::F_luminance:
      tex->set_format(Texture::F_luminance);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2pg_cat.warning()
        << "Ignoring inappropriate format " << egg_tex->get_format()
        << " for 1-component texture " << egg_tex->get_name() << "\n";
    }

  } else if (tex->get_num_components() == 2) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_luminance_alpha:
      tex->set_format(Texture::F_luminance_alpha);
      break;

    case EggTexture::F_luminance_alphamask:
      tex->set_format(Texture::F_luminance_alphamask);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2pg_cat.warning()
        << "Ignoring inappropriate format " << egg_tex->get_format()
        << " for 2-component texture " << egg_tex->get_name() << "\n";
    }

  } else if (tex->get_num_components() == 3) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_rgb:
      tex->set_format(Texture::F_rgb);
      break;
    case EggTexture::F_rgb12:
      if (tex->get_component_width() >= 2) {
        // Only do this if the component width supports it.
        tex->set_format(Texture::F_rgb12);
      } else {
        egg2pg_cat.warning()
          << "Ignoring inappropriate format " << egg_tex->get_format()
          << " for 8-bit texture " << egg_tex->get_name() << "\n";
      }
      break;
    case EggTexture::F_rgb8:
    case EggTexture::F_rgba8:
      // We'll quietly accept RGBA8 for a 3-component texture, since flt2egg
      // generates these for 3-component as well as for 4-component textures.
      tex->set_format(Texture::F_rgb8);
      break;
    case EggTexture::F_rgb5:
      tex->set_format(Texture::F_rgb5);
      break;
    case EggTexture::F_rgb332:
      tex->set_format(Texture::F_rgb332);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2pg_cat.warning()
        << "Ignoring inappropriate format " << egg_tex->get_format()
        << " for 3-component texture " << egg_tex->get_name() << "\n";
    }

  } else if (tex->get_num_components() == 4) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_rgba:
      tex->set_format(Texture::F_rgba);
      break;
    case EggTexture::F_rgbm:
      tex->set_format(Texture::F_rgbm);
      break;
    case EggTexture::F_rgba12:
      if (tex->get_component_width() >= 2) {
        // Only do this if the component width supports it.
        tex->set_format(Texture::F_rgba12);
      } else {
        egg2pg_cat.warning()
          << "Ignoring inappropriate format " << egg_tex->get_format()
          << " for 8-bit texture " << egg_tex->get_name() << "\n";
      }
      break;
    case EggTexture::F_rgba8:
      tex->set_format(Texture::F_rgba8);
      break;
    case EggTexture::F_rgba4:
      tex->set_format(Texture::F_rgba4);
      break;
    case EggTexture::F_rgba5:
      tex->set_format(Texture::F_rgba5);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2pg_cat.warning()
        << "Ignoring inappropriate format " << egg_tex->get_format()
        << " for 4-component texture " << egg_tex->get_name() << "\n";
    }
  }

  switch (egg_tex->get_quality_level()) {
  case EggTexture::QL_unspecified:
  case EggTexture::QL_default:
    tex->set_quality_level(Texture::QL_default);
    break;

  case EggTexture::QL_fastest:
    tex->set_quality_level(Texture::QL_fastest);
    break;

  case EggTexture::QL_normal:
    tex->set_quality_level(Texture::QL_normal);
    break;

  case EggTexture::QL_best:
    tex->set_quality_level(Texture::QL_best);
    break;
  }
}

/**
 * Returns the Texture::CompressionMode enum corresponding to the
 * EggTexture::CompressionMode.  Returns CM_default if the compression mode is
 * unspecified.
 */
Texture::CompressionMode EggLoader::
convert_compression_mode(EggTexture::CompressionMode compression_mode) const {
  switch (compression_mode) {
  case EggTexture::CM_off:
    return Texture::CM_off;

  case EggTexture::CM_on:
    return Texture::CM_on;

  case EggTexture::CM_fxt1:
    return Texture::CM_fxt1;

  case EggTexture::CM_dxt1:
    return Texture::CM_dxt1;

  case EggTexture::CM_dxt2:
    return Texture::CM_dxt2;

  case EggTexture::CM_dxt3:
    return Texture::CM_dxt3;

  case EggTexture::CM_dxt4:
    return Texture::CM_dxt4;

  case EggTexture::CM_dxt5:
    return Texture::CM_dxt5;

  case EggTexture::CM_default:
    return Texture::CM_default;
  }

  egg2pg_cat.warning()
    << "Unexpected texture compression flag: " << (int)compression_mode << "\n";
  return Texture::CM_default;
}

/**
 * Returns the SamplerState::WrapMode enum corresponding to the
 * EggTexture::WrapMode.  Returns WM_repeat if the wrap mode is unspecified.
 */
SamplerState::WrapMode EggLoader::
convert_wrap_mode(EggTexture::WrapMode wrap_mode) const {
  switch (wrap_mode) {
  case EggTexture::WM_clamp:
    return SamplerState::WM_clamp;

  case EggTexture::WM_repeat:
    return SamplerState::WM_repeat;

  case EggTexture::WM_mirror:
    return SamplerState::WM_mirror;

  case EggTexture::WM_mirror_once:
    return SamplerState::WM_mirror_once;

  case EggTexture::WM_border_color:
    return SamplerState::WM_border_color;

  case EggTexture::WM_unspecified:
    return SamplerState::WM_repeat;
  }

  egg2pg_cat.warning()
    << "Unexpected texture wrap flag: " << (int)wrap_mode << "\n";
  return SamplerState::WM_repeat;
}

/**
 * Creates a TextureStage object suitable for rendering the indicated texture.
 */
PT(TextureStage) EggLoader::
make_texture_stage(const EggTexture *egg_tex) {
  // If the egg texture specifies any relevant TextureStage properties, or if
  // it is multitextured on top of anything else, it gets its own texture
  // stage; otherwise, it gets the default texture stage.
  if (!egg_tex->has_stage_name() &&
      !egg_tex->has_uv_name() &&
      !egg_tex->has_color() &&
      (egg_tex->get_env_type() == EggTexture::ET_unspecified ||
       egg_tex->get_env_type() == EggTexture::ET_modulate) &&
      egg_tex->get_combine_mode(EggTexture::CC_rgb) == EggTexture::CM_unspecified &&
      egg_tex->get_combine_mode(EggTexture::CC_alpha) == EggTexture::CM_unspecified &&

      !egg_tex->has_priority() &&
      egg_tex->get_multitexture_sort() == 0 &&
      !egg_tex->get_saved_result()) {
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

  case EggTexture::ET_blend_color_scale:
    stage->set_mode(TextureStage::M_blend_color_scale);
    break;

  case EggTexture::ET_modulate_glow:
    stage->set_mode(TextureStage::M_modulate_glow);
    break;

  case EggTexture::ET_modulate_gloss:
    stage->set_mode(TextureStage::M_modulate_gloss);
    break;

  case EggTexture::ET_normal:
    stage->set_mode(TextureStage::M_normal);
    break;

  case EggTexture::ET_normal_height:
    stage->set_mode(TextureStage::M_normal_height);
    break;

  case EggTexture::ET_glow:
    stage->set_mode(TextureStage::M_glow);
    break;

  case EggTexture::ET_gloss:
    stage->set_mode(TextureStage::M_gloss);
    break;

  case EggTexture::ET_height:
    stage->set_mode(TextureStage::M_height);
    break;

  case EggTexture::ET_selector:
    stage->set_mode(TextureStage::M_selector);
    break;

  case EggTexture::ET_normal_gloss:
    stage->set_mode(TextureStage::M_normal_gloss);
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


  if (egg_tex->has_uv_name()) {
    PT(InternalName) name =
      InternalName::get_texcoord_name(egg_tex->get_uv_name());
    stage->set_texcoord_name(name);
  }

  if (egg_tex->has_rgb_scale()) {
    stage->set_rgb_scale(egg_tex->get_rgb_scale());
  }

  if (egg_tex->has_alpha_scale()) {
    stage->set_alpha_scale(egg_tex->get_alpha_scale());
  }

  stage->set_saved_result(egg_tex->get_saved_result());

  stage->set_sort(egg_tex->get_multitexture_sort() * 10);

  if (egg_tex->has_priority()) {
    stage->set_sort(egg_tex->get_priority());
  }

  if (egg_tex->has_color()) {
    stage->set_color(egg_tex->get_color());
  }

  return TextureStagePool::get_stage(stage);
}

/**
 * Walks the tree recursively, looking for EggPrimitives that are children of
 * sequence or switch nodes.  If any are found, they are moved within their
 * own group to protect them from being flattened with their neighbors.
 */
void EggLoader::
separate_switches(EggNode *egg_node) {
  bool parent_has_switch = false;
  if (egg_node->is_of_type(EggGroup::get_class_type())) {
    EggGroup *egg_group = DCAST(EggGroup, egg_node);
    parent_has_switch = egg_group->get_switch_flag();
  }

  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);

    EggGroupNode::iterator ci;
    ci = egg_group->begin();
    while (ci != egg_group->end()) {
      EggGroupNode::iterator cnext;
      cnext = ci;
      ++cnext;

      PT(EggNode) child = (*ci);
      if (parent_has_switch &&
          child->is_of_type(EggPrimitive::get_class_type())) {
        // Move this child under a new node.
        PT(EggGroup) new_group = new EggGroup(child->get_name());
        egg_group->replace(ci, new_group.p());
        new_group->add_child(child);
      }

      separate_switches(child);

      ci = cnext;
    }
  }
}

/**
 * Looks for EggPolygons with a bface flag applied to them.  Any such polygons
 * are duplicated into a pair of back-to-back polygons, and the bface flag is
 * removed.
 */
void EggLoader::
emulate_bface(EggNode *egg_node) {
  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);
    PT(EggGroupNode) dup_prims = new EggGroupNode;

    EggGroupNode::iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      PT(EggNode) child = (*ci);
      if (child->is_of_type(EggPolygon::get_class_type())) {
        EggPolygon *poly = DCAST(EggPolygon, child);
        if (poly->get_bface_flag()) {
          poly->set_bface_flag(false);

          PT(EggPolygon) dup_poly = new EggPolygon(*poly);
          dup_poly->reverse_vertex_ordering();
          if (dup_poly->has_normal()) {
            dup_poly->set_normal(-dup_poly->get_normal());
          }

          // Also reverse the normal on any vertices.
          EggPolygon::iterator vi;
          for (vi = dup_poly->begin(); vi != dup_poly->end(); ++vi) {
            EggVertex *vertex = (*vi);
            if (vertex->has_normal()) {
              EggVertex dup_vertex(*vertex);
              dup_vertex.set_normal(-dup_vertex.get_normal());
              EggVertex *new_vertex = vertex->get_pool()->create_unique_vertex(dup_vertex);
              if (new_vertex != vertex) {
                new_vertex->copy_grefs_from(*vertex);
                dup_poly->replace(vi, new_vertex);
              }
            }
          }
          dup_prims->add_child(dup_poly);
        }
      }

      emulate_bface(child);
    }

    // Now that we've iterated through all the children, add in any duplicated
    // polygons we generated.
    egg_group->steal_children(*dup_prims);
  }
}

/**
 *
 */
PandaNode *EggLoader::
make_node(EggNode *egg_node, PandaNode *parent) {
  if (egg_node->is_of_type(EggBin::get_class_type())) {
    return make_node(DCAST(EggBin, egg_node), parent);
  } else if (egg_node->is_of_type(EggGroup::get_class_type())) {
    return make_node(DCAST(EggGroup, egg_node), parent);
  } else if (egg_node->is_of_type(EggTable::get_class_type())) {
    return make_node(DCAST(EggTable, egg_node), parent);
  } else if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    return make_node(DCAST(EggGroupNode, egg_node), parent);
  }

  return nullptr;
}

/**
 *
 */
PandaNode *EggLoader::
make_node(EggBin *egg_bin, PandaNode *parent) {
  // An EggBin might mean an LOD node (i.e.  a parent of one or more EggGroups
  // with LOD specifications), or it might mean a polyset node (a parent of
  // one or more similar EggPrimitives).
  switch (egg_bin->get_bin_number()) {
  case EggBinner::BN_polyset:
  case EggBinner::BN_patches:
    make_polyset(egg_bin, parent, nullptr, _dynamic_override, _dynamic_override_char_maker);
    return nullptr;

  case EggBinner::BN_lod:
    return make_lod(egg_bin, parent);

  case EggBinner::BN_nurbs_surface:
    {
      nassertr(!egg_bin->empty(), nullptr);
      EggNode *child = egg_bin->get_first_child();
      EggNurbsSurface *egg_nurbs;
      DCAST_INTO_R(egg_nurbs, child, nullptr);
      const LMatrix4d &mat = egg_nurbs->get_vertex_to_node();
      make_nurbs_surface(egg_nurbs, parent, mat);
    }
    return nullptr;

  case EggBinner::BN_nurbs_curve:
    {
      nassertr(!egg_bin->empty(), nullptr);
      EggNode *child = egg_bin->get_first_child();
      EggNurbsCurve *egg_nurbs;
      DCAST_INTO_R(egg_nurbs, child, nullptr);
      const LMatrix4d &mat = egg_nurbs->get_vertex_to_node();
      make_nurbs_curve(egg_nurbs, parent, mat);
    }
    return nullptr;

  case EggBinner::BN_none:
    break;
  }

  // Shouldn't get here.
  return nullptr;
}

/**
 *
 */
PandaNode *EggLoader::
make_lod(EggBin *egg_bin, PandaNode *parent) {
  PT(LODNode) lod_node = LODNode::make_default_lod(egg_bin->get_name());

  pvector<LODInstance> instances;

  EggGroup::const_iterator ci;
  for (ci = egg_bin->begin(); ci != egg_bin->end(); ++ci) {
    LODInstance instance(*ci);
    instances.push_back(instance);
  }

  // Now that we've created all of our children, put them in the proper order
  // and tell the LOD node about them.
  sort(instances.begin(), instances.end());

  if (!instances.empty()) {
    // Set up the LOD node's center.  All of the children should have the same
    // center, because that's how we binned them.
    lod_node->set_center(LCAST(PN_stdfloat, instances[0]._d->_center));
  }

  for (size_t i = 0; i < instances.size(); i++) {
    // Create the children in the proper order within the scene graph.
    const LODInstance &instance = instances[i];
    make_node(instance._egg_node, lod_node);

    // All of the children should have the same center, because that's how we
    // binned them.
    nassertr(lod_node->get_center().almost_equal
             (LCAST(PN_stdfloat, instance._d->_center), 0.01), nullptr);

    // Tell the LOD node about this child's switching distances.
    lod_node->add_switch(instance._d->_switch_in, instance._d->_switch_out);
  }

  _groups[egg_bin] = lod_node;
  return create_group_arc(egg_bin, parent, lod_node);
}

/**
 *
 */
PandaNode *EggLoader::
make_node(EggGroup *egg_group, PandaNode *parent) {
  PT(PandaNode) node = nullptr;

  if (egg_group->get_dart_type() != EggGroup::DT_none) {
    // A group with the <Dart> flag set means to create a character.
    bool structured = (egg_group->get_dart_type() == EggGroup::DT_structured);

    CharacterMaker char_maker(egg_group, *this, structured);

    node = char_maker.make_node();
    if(structured) {
      // we're going to generate the rest of the children normally except
      // we'll be making dynamic geometry
      _dynamic_override = true;
      _dynamic_override_char_maker = &char_maker;
      EggGroup::const_iterator ci;
      for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
        make_node(*ci, node);
      }
      _dynamic_override_char_maker = nullptr;
      _dynamic_override = false;
    }

  } else if (egg_group->get_cs_type() != EggGroup::CST_none) {
    // A collision group: create collision geometry.
    node = new CollisionNode(egg_group->get_name());

    // Piggy-back the desired transform to apply onto the node, since we can't
    // break the ABI in 1.10.
    node->set_transform(TransformState::make_mat(LCAST(PN_stdfloat, egg_group->get_vertex_to_node())));
    make_collision_solids(egg_group, egg_group, (CollisionNode *)node.p());
    node->clear_transform();

    if ((egg_group->get_collide_flags() & EggGroup::CF_keep) != 0) {
      // If we also specified to keep the geometry, continue the traversal.
      // In this case, we create a new PandaNode to be the parent of the
      // visible geometry and the collision geometry.
      PandaNode *combined = new PandaNode("");
      parent->add_child(combined);
      combined->add_child(node);
      node = combined;

      EggGroup::const_iterator ci;
      for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
        make_node(*ci, combined);
      }
    }

    node = create_group_arc(egg_group, parent, node);
    return node;

  } else if (egg_group->get_portal_flag()) {
    // Create a portal instead of a regular polyset.  Scan the children of
    // this node looking for a polygon, similar to the collision polygon case,
    // above.
    PortalNode *pnode = new PortalNode(egg_group->get_name());
    node = pnode;

    set_portal_polygon(egg_group, pnode);
    if (pnode->get_num_vertices() == 0) {
      egg2pg_cat.warning()
        << "Portal " << egg_group->get_name() << " has no vertices!\n";
    }

  } else if (egg_group->get_occluder_flag()) {
    // Create an occluder instead of a regular polyset.  Scan the children of
    // this node looking for a polygon, the same as the portal polygon case,
    // above.
    OccluderNode *pnode = new OccluderNode(egg_group->get_name());
    node = pnode;

    set_occluder_polygon(egg_group, pnode);
    if (pnode->get_num_vertices() == 0) {
      egg2pg_cat.warning()
        << "Occluder " << egg_group->get_name() << " has no vertices!\n";
    }

  } else if (egg_group->get_polylight_flag()) {
    // Create a polylight instead of a regular polyset.  use make_sphere to
    // get the center, radius and color egg2pg_cat.debug() << "polylight
    // node\n";
    LPoint3 center;
    LColor color;
    PN_stdfloat radius;

    if (!make_sphere(egg_group, EggGroup::CF_none, center, radius, color)) {
      egg2pg_cat.warning()
        << "Polylight " << egg_group->get_name() << " make_sphere failed!\n";
    }
    PolylightNode *pnode = new PolylightNode(egg_group->get_name());
    pnode->set_pos(center);
    pnode->set_color(color);
    pnode->set_radius(radius);

    pnode->xform(LCAST(PN_stdfloat, egg_group->get_vertex_to_node()));

    node = pnode;

  } else if (egg_group->get_switch_flag()) {
    if (egg_group->get_switch_fps() != 0.0) {
      // Create a sequence node.
      node = new SequenceNode(egg_group->get_name());
      ((SequenceNode *)node.p())->set_frame_rate(egg_group->get_switch_fps());
      _sequences.insert(node);
    } else {
      // Create a switch node.
      node = new SwitchNode(egg_group->get_name());
    }

    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }
  } else if (egg_group->has_scrolling_uvs()) {
    node = new UvScrollNode(egg_group->get_name(), egg_group->get_scroll_u(), egg_group->get_scroll_v(), egg_group->get_scroll_w(), egg_group->get_scroll_r());

    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }

  } else if (egg_group->get_model_flag() || egg_group->has_dcs_type()) {
    // A model or DCS flag; create a model node.
    node = new ModelNode(egg_group->get_name());
    switch (egg_group->get_dcs_type()) {
    case EggGroup::DC_net:
      DCAST(ModelNode, node)->set_preserve_transform(ModelNode::PT_net);
      break;

    case EggGroup::DC_no_touch:
      DCAST(ModelNode, node)->set_preserve_transform(ModelNode::PT_no_touch);
      break;

    case EggGroup::DC_local:
    case EggGroup::DC_default:
      DCAST(ModelNode, node)->set_preserve_transform(ModelNode::PT_local);
      break;

    case EggGroup::DC_none:
    case EggGroup::DC_unspecified:
      break;
    }

    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }

  } else {
    // A normal group; just create a normal node, and traverse.  But if all of
    // the children of this group are polysets, anticipate this for the
    // benefit of smaller grouping, and create a single GeomNode for all of
    // the children.
    bool all_polysets = false;
    bool any_hidden = false;

    // We don't want to ever create a GeomNode under a "decal" flag, since
    // that can confuse the decal reparenting.
    if (!egg_group->determine_decal()) {
      check_for_polysets(egg_group, all_polysets, any_hidden);
    }

    if (all_polysets && !any_hidden) {
      node = new GeomNode(egg_group->get_name());
    } else {
      node = new PandaNode(egg_group->get_name());
    }

    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }
  }

  if (node == nullptr) {
    return nullptr;
  }

  // Associate any instances with this node.
  int num_group_refs = egg_group->get_num_group_refs();
  for (int gri = 0; gri < num_group_refs; ++gri) {
    EggGroup *group_ref = egg_group->get_group_ref(gri);
    Groups::const_iterator gi = _groups.find(group_ref);
    if (gi != _groups.end()) {
      PandaNode *node_ref = (*gi).second;
      node->add_child(node_ref);
    }
  }

  _groups[egg_group] = node;
  return create_group_arc(egg_group, parent, node);
}

/**
 * Creates the arc parenting a new group to the scene graph, and applies any
 * relevant attribs to the arc according to the EggGroup node that inspired
 * the group.
 */
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
    // descendant groups will be decaled onto the geometry within this group.
    // This means we'll need to reparent things a bit afterward.
    _decals.insert(node);
  }

  // Copy all the tags from the group onto the node.
  EggGroup::TagData::const_iterator ti;
  for (ti = egg_group->tag_begin(); ti != egg_group->tag_end(); ++ti) {
    node->set_tag((*ti).first, (*ti).second);
  }

  if (egg_group->get_blend_mode() != EggGroup::BM_unspecified &&
      egg_group->get_blend_mode() != EggGroup::BM_none) {
    // Apply a ColorBlendAttrib to the group.
    ColorBlendAttrib::Mode mode = get_color_blend_mode(egg_group->get_blend_mode());
    ColorBlendAttrib::Operand a = get_color_blend_operand(egg_group->get_blend_operand_a());
    ColorBlendAttrib::Operand b = get_color_blend_operand(egg_group->get_blend_operand_b());
    LColor color = egg_group->get_blend_color();
    node->set_attrib(ColorBlendAttrib::make(mode, a, b, color));
  }

  // If the group specified some property that should propagate down to the
  // leaves, we have to remember this node and apply the property later, after
  // we've created the actual geometry.
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

/**
 *
 */
PandaNode *EggLoader::
make_node(EggTable *egg_table, PandaNode *parent) {
  if (egg_table->get_table_type() != EggTable::TT_bundle) {
    // We only do anything with bundles.  Isolated tables are treated as
    // ordinary groups.
    return make_node(DCAST(EggGroupNode, egg_table), parent);
  }

  // It's an actual bundle, so make an AnimBundle from it and its descendants.
  AnimBundleMaker bundle_maker(egg_table);
  AnimBundleNode *node = bundle_maker.make_node();
  parent->add_child(node);
  return node;
}


/**
 *
 */
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

/**
 * Sets all_polysets true if all of the children of this node represent a
 * polyset.  Sets any_hidden true if any of those polysets are flagged hidden.
 */
void EggLoader::
check_for_polysets(EggGroup *egg_group, bool &all_polysets, bool &any_hidden) {
  all_polysets = (!egg_group->empty());
  any_hidden = false;

  EggGroup::const_iterator ci;
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggBin::get_class_type())) {
      EggBin *egg_bin = DCAST(EggBin, (*ci));
      if (egg_bin->get_bin_number() == EggBinner::BN_polyset) {
        // We know that all of the primitives in the bin have the same render
        // state, so we can get that information from the first primitive.
        EggGroup::const_iterator bci = egg_bin->begin();
        nassertv(bci != egg_bin->end());
        const EggPrimitive *first_prim;
        DCAST_INTO_V(first_prim, (*bci));
        const EggRenderState *render_state;
        DCAST_INTO_V(render_state, first_prim->get_user_data(EggRenderState::get_class_type()));

        if (render_state->_hidden) {
          any_hidden = true;
        }
      } else {
        all_polysets = false;
        return;
      }
    } else if ((*ci)->is_of_type(EggGroup::get_class_type())) {
      // Other kinds of children, like vertex pools, comments, textures, etc.,
      // are ignored; but groups indicate more nodes, so if we find a nested
      // group it means we're not all polysets.
      all_polysets = false;
      return;
    }
  }
}

/**
 * Creates a GeomVertexData structure from the vertex pool, for the indicated
 * transform space.  If a GeomVertexData has already been created for this
 * transform, just returns it.
 */
PT(GeomVertexData) EggLoader::
make_vertex_data(const EggRenderState *render_state,
                 EggVertexPool *vertex_pool, EggNode *primitive_home,
                 const LMatrix4d &transform, TransformBlendTable *blend_table,
                 bool is_dynamic, CharacterMaker *character_maker,
                 bool ignore_color) {
  VertexPoolTransform vpt;
  vpt._vertex_pool = vertex_pool;
  vpt._bake_in_uvs = render_state->_bake_in_uvs;
  vpt._transform = transform;

  VertexPoolData::iterator di;
  di = _vertex_pool_data.find(vpt);
  if (di != _vertex_pool_data.end()) {
    return (*di).second;
  }

  PT(GeomVertexArrayFormat) array_format = new GeomVertexArrayFormat;
  array_format->add_column
    (InternalName::get_vertex(), vertex_pool->get_num_dimensions(),
     Geom::NT_stdfloat, Geom::C_point);

  if (vertex_pool->has_normals()) {
    array_format->add_column
      (InternalName::get_normal(), 3,
       Geom::NT_stdfloat, Geom::C_normal);
  }

  if (!ignore_color) {
    // Let's not use Direct3D-style colors on platforms where we only have
    // OpenGL anyway.
#ifdef _WIN32
    array_format->add_column(InternalName::get_color(), 1,
                             Geom::NT_packed_dabc, Geom::C_color);
#else
    array_format->add_column(InternalName::get_color(), 4,
                             Geom::NT_uint8, Geom::C_color);
#endif
  }

  vector_string uv_names, uvw_names, tbn_names;
  vertex_pool->get_uv_names(uv_names, uvw_names, tbn_names);
  vector_string::const_iterator ni;
  for (ni = uv_names.begin(); ni != uv_names.end(); ++ni) {
    string name = (*ni);

    PT(InternalName) iname = InternalName::get_texcoord_name(name);

    if (find(uvw_names.begin(), uvw_names.end(), name) != uvw_names.end()) {
      // This one actually represents 3-d texture coordinates.
      array_format->add_column
        (iname, 3, Geom::NT_stdfloat, Geom::C_texcoord);
    } else {
      array_format->add_column
        (iname, 2, Geom::NT_stdfloat, Geom::C_texcoord);
    }
  }
  for (ni = tbn_names.begin(); ni != tbn_names.end(); ++ni) {
    string name = (*ni);

    PT(InternalName) iname_t = InternalName::get_tangent_name(name);
    PT(InternalName) iname_b = InternalName::get_binormal_name(name);

    array_format->add_column
      (iname_t, 3, Geom::NT_stdfloat, Geom::C_vector);
    array_format->add_column
      (iname_b, 3, Geom::NT_stdfloat, Geom::C_vector);
  }

  vector_string aux_names;
  vertex_pool->get_aux_names(aux_names);
  for (ni = aux_names.begin(); ni != aux_names.end(); ++ni) {
    string name = (*ni);
    PT(InternalName) iname = InternalName::make(name);
    array_format->add_column
      (iname, 4, Geom::NT_stdfloat, Geom::C_other);
  }

  PT(GeomVertexFormat) temp_format = new GeomVertexFormat(array_format);

  PT(SliderTable) slider_table;
  string name = _data->get_egg_filename().get_basename_wo_extension();

  if (is_dynamic) {
    // If it's a dynamic object, we need a TransformBlendTable and maybe a
    // SliderTable, and additional columns in the vertex data: one that
    // indexes into the blend table per vertex, and also one for each
    // different type of morph delta.

    // Tell the format that we're setting it up for Panda-based animation.
    GeomVertexAnimationSpec animation;
    animation.set_panda();
    temp_format->set_animation(animation);

    PT(GeomVertexArrayFormat) anim_array_format = new GeomVertexArrayFormat;
    anim_array_format->add_column
      (InternalName::get_transform_blend(), 1,
       Geom::NT_uint16, Geom::C_index, 0, 2);
    temp_format->add_array(anim_array_format);

    pmap<string, BitArray> slider_names;
    EggVertexPool::const_iterator vi;
    for (vi = vertex_pool->begin(); vi != vertex_pool->end(); ++vi) {
      EggVertex *vertex = (*vi);

      EggMorphVertexList::const_iterator mvi;
      for (mvi = vertex->_dxyzs.begin(); mvi != vertex->_dxyzs.end(); ++mvi) {
        slider_names[(*mvi).get_name()].set_bit(vertex->get_index());
        record_morph(anim_array_format, character_maker, (*mvi).get_name(),
                     InternalName::get_vertex(), 3);
      }
      if (vertex->has_normal()) {
        EggMorphNormalList::const_iterator mni;
        for (mni = vertex->_dnormals.begin(); mni != vertex->_dnormals.end(); ++mni) {
          slider_names[(*mni).get_name()].set_bit(vertex->get_index());
          record_morph(anim_array_format, character_maker, (*mni).get_name(),
                       InternalName::get_normal(), 3);
        }
      }
      if (!ignore_color && vertex->has_color()) {
        EggMorphColorList::const_iterator mci;
        for (mci = vertex->_drgbas.begin(); mci != vertex->_drgbas.end(); ++mci) {
          slider_names[(*mci).get_name()].set_bit(vertex->get_index());
          record_morph(anim_array_format, character_maker, (*mci).get_name(),
                       InternalName::get_color(), 4);
        }
      }
      EggVertex::const_uv_iterator uvi;
      for (uvi = vertex->uv_begin(); uvi != vertex->uv_end(); ++uvi) {
        EggVertexUV *egg_uv = (*uvi);
        string name = egg_uv->get_name();
        bool has_w = (find(uvw_names.begin(), uvw_names.end(), name) != uvw_names.end());
        PT(InternalName) iname = InternalName::get_texcoord_name(name);

        EggMorphTexCoordList::const_iterator mti;
        for (mti = egg_uv->_duvs.begin(); mti != egg_uv->_duvs.end(); ++mti) {
          slider_names[(*mti).get_name()].set_bit(vertex->get_index());
          record_morph(anim_array_format, character_maker, (*mti).get_name(),
                       iname, has_w ? 3 : 2);
        }
      }
    }

    if (!slider_names.empty()) {
      // If we have any sliders at all, create a table for them.

      slider_table = new SliderTable;

      pmap<string, BitArray>::iterator si;
      for (si = slider_names.begin(); si != slider_names.end(); ++si) {
        PT(VertexSlider) slider = character_maker->egg_to_slider((*si).first);
        slider_table->add_slider(slider, (*si).second);
      }
    }

    // We'll also assign the character name to the vertex data, so it will
    // show up in PStats.
    name = character_maker->get_name();
  }

  temp_format->maybe_align_columns_for_animation();

  CPT(GeomVertexFormat) format =
    GeomVertexFormat::register_format(temp_format);

  // Now create a new GeomVertexData using the indicated format.  It is
  // actually correct to create it with UH_static even though it represents a
  // dynamic object, because the vertex data itself won't be changing--just
  // the result of applying the animation is dynamic.
  PT(GeomVertexData) vertex_data =
    new GeomVertexData(name, format, Geom::UH_static);
  vertex_data->reserve_num_rows(vertex_pool->size());

  vertex_data->set_transform_blend_table(blend_table);
  if (slider_table != nullptr) {
    vertex_data->set_slider_table(SliderTable::register_table(slider_table));
  }

  // And fill in the data from the vertex pool.
  EggVertexPool::const_iterator vi;
  for (vi = vertex_pool->begin(); vi != vertex_pool->end(); ++vi) {
    GeomVertexWriter gvw(vertex_data);
    EggVertex *vertex = (*vi);
    gvw.set_row(vertex->get_index());

    gvw.set_column(InternalName::get_vertex());
    gvw.add_data4d(vertex->get_pos4() * transform);

    if (is_dynamic) {
      EggMorphVertexList::const_iterator mvi;
      for (mvi = vertex->_dxyzs.begin(); mvi != vertex->_dxyzs.end(); ++mvi) {
        const EggMorphVertex &morph = (*mvi);
        CPT(InternalName) delta_name =
          InternalName::get_morph(InternalName::get_vertex(), morph.get_name());
        gvw.set_column(delta_name);
        gvw.add_data3d(morph.get_offset() * transform);
      }
    }

    if (vertex->has_normal()) {
      gvw.set_column(InternalName::get_normal());
      LNormald orig_normal = vertex->get_normal();
      LNormald transformed_normal = normalize(orig_normal * transform);
      gvw.add_data3d(transformed_normal);

      if (is_dynamic) {
        EggMorphNormalList::const_iterator mni;
        for (mni = vertex->_dnormals.begin(); mni != vertex->_dnormals.end(); ++mni) {
          const EggMorphNormal &morph = (*mni);
          CPT(InternalName) delta_name =
            InternalName::get_morph(InternalName::get_normal(), morph.get_name());
          gvw.set_column(delta_name);
          LNormald morphed_normal = orig_normal + morph.get_offset();
          LNormald transformed_morphed_normal = normalize(morphed_normal * transform);
          LVector3d delta = transformed_morphed_normal - transformed_normal;
          gvw.add_data3d(delta);
        }
      }
    }

    if (!ignore_color && vertex->has_color()) {
      gvw.set_column(InternalName::get_color());
      gvw.add_data4(vertex->get_color());

      if (is_dynamic) {
        EggMorphColorList::const_iterator mci;
        for (mci = vertex->_drgbas.begin(); mci != vertex->_drgbas.end(); ++mci) {
          const EggMorphColor &morph = (*mci);
          CPT(InternalName) delta_name =
            InternalName::get_morph(InternalName::get_color(), morph.get_name());
          gvw.set_column(delta_name);
          gvw.add_data4(morph.get_offset());
        }
      }
    }

    EggVertex::const_uv_iterator uvi;
    for (uvi = vertex->uv_begin(); uvi != vertex->uv_end(); ++uvi) {
      EggVertexUV *egg_uv = (*uvi);
      LTexCoord3d orig_uvw = egg_uv->get_uvw();
      LTexCoord3d uvw = egg_uv->get_uvw();

      string name = egg_uv->get_name();
      PT(InternalName) iname = InternalName::get_texcoord_name(name);
      gvw.set_column(iname);

      BakeInUVs::const_iterator buv = render_state->_bake_in_uvs.find(iname);
      if (buv != render_state->_bake_in_uvs.end()) {
        // If we are to bake in a texture matrix, do so now.
        uvw = uvw * (*buv).second->get_transform3d();
      }

      gvw.set_data3d(uvw);

      if (is_dynamic) {
        EggMorphTexCoordList::const_iterator mti;
        for (mti = egg_uv->_duvs.begin(); mti != egg_uv->_duvs.end(); ++mti) {
          const EggMorphTexCoord &morph = (*mti);
          CPT(InternalName) delta_name =
            InternalName::get_morph(iname, morph.get_name());
          gvw.set_column(delta_name);
          LTexCoord3d duvw = morph.get_offset();
          if (buv != render_state->_bake_in_uvs.end()) {
            LTexCoord3d new_uvw = orig_uvw + duvw;
            duvw = (new_uvw * (*buv).second->get_transform3d()) - uvw;
          }

          gvw.add_data3d(duvw);
        }
      }

      // Also add the tangent and binormal, if present.
      if (egg_uv->has_tangent() && egg_uv->has_binormal()) {
        PT(InternalName) iname = InternalName::get_tangent_name(name);
        gvw.set_column(iname);
        if (gvw.has_column()) {
          LVector3d tangent = egg_uv->get_tangent();
          LVector3d binormal = egg_uv->get_binormal();
          gvw.add_data3d(tangent);
          gvw.set_column(InternalName::get_binormal_name(name));
          gvw.add_data3d(binormal);
        }
      }
    }

    EggVertex::const_aux_iterator auxi;
    for (auxi = vertex->aux_begin(); auxi != vertex->aux_end(); ++auxi) {
      EggVertexAux *egg_aux = (*auxi);
      LVecBase4d aux = egg_aux->get_aux();

      string name = egg_aux->get_name();
      PT(InternalName) iname = InternalName::make(name);
      gvw.set_column(iname);

      gvw.set_data4d(aux);
    }

    if (is_dynamic) {
      int table_index = vertex->get_external_index();
      gvw.set_column(InternalName::get_transform_blend());
      gvw.set_data1i(table_index);
    }
  }

  bool inserted = _vertex_pool_data.insert
    (VertexPoolData::value_type(vpt, vertex_data)).second;
  nassertr(inserted, vertex_data);

  Thread::consider_yield();
  return vertex_data;
}

/**
 *
 */
PT(TransformBlendTable) EggLoader::
make_blend_table(EggVertexPool *vertex_pool, EggNode *primitive_home,
                 CharacterMaker *character_maker) {
  PT(TransformBlendTable) blend_table;
  blend_table = new TransformBlendTable;
  blend_table->set_rows(SparseArray::lower_on(vertex_pool->size()));

  EggVertexPool::const_iterator vi;
  for (vi = vertex_pool->begin(); vi != vertex_pool->end(); ++vi) {
    EggVertex *vertex = (*vi);

    // Figure out the transforms affecting this particular vertex.
    TransformBlend blend;
    if (vertex->gref_size() == 0) {
      // If the vertex has no explicit membership, it belongs right where it
      // is.
      PT(VertexTransform) vt = character_maker->egg_to_transform(primitive_home);
      nassertr(vt != nullptr, nullptr);
      blend.add_transform(vt, 1.0f);
    } else {
      // If the vertex does have an explicit membership, ignore its parentage
      // and assign it where it wants to be.
      double quantize = egg_vertex_membership_quantize;
      EggVertex::GroupRef::const_iterator gri;
      for (gri = vertex->gref_begin(); gri != vertex->gref_end(); ++gri) {
        EggGroup *egg_joint = (*gri);
        double membership = egg_joint->get_vertex_membership(vertex);
        if (quantize != 0.0) {
          membership = cfloor(membership / quantize + 0.5) * quantize;
        }

        PT(VertexTransform) vt = character_maker->egg_to_transform(egg_joint);
        nassertr(vt != nullptr, nullptr);
        blend.add_transform(vt, membership);
      }
    }
    if (egg_vertex_max_num_joints >= 0) {
      blend.limit_transforms(egg_vertex_max_num_joints);
    }
    blend.normalize_weights();

    int table_index = blend_table->add_blend(blend);

    // We take advantage of the "external index" field of the EggVertex to
    // temporarily store the transform blend index.
    vertex->set_external_index(table_index);
  }

  return blend_table;
}

/**
 *
 */
void EggLoader::
record_morph(GeomVertexArrayFormat *array_format,
             CharacterMaker *character_maker,
             const string &morph_name, InternalName *column_name,
             int num_components) {
  PT(InternalName) delta_name =
    InternalName::get_morph(column_name, morph_name);
  if (!array_format->has_column(delta_name)) {
    array_format->add_column
      (delta_name, num_components,
       Geom::NT_stdfloat, Geom::C_morph_delta);
  }
}

/**
 * Creates a GeomPrimitive corresponding to the indicated EggPrimitive, and
 * adds it to the set.
 */
void EggLoader::
make_primitive(const EggRenderState *render_state, EggPrimitive *egg_prim,
               EggLoader::UniquePrimitives &unique_primitives,
               EggLoader::Primitives &primitives,
               bool has_overall_color, const LColor &overall_color) {
  PT(GeomPrimitive) primitive;
  if (egg_prim->is_of_type(EggPolygon::get_class_type())) {
    if (egg_prim->size() == 3) {
      primitive = new GeomTriangles(Geom::UH_static);
    }

  } else if (egg_prim->is_of_type(EggTriangleStrip::get_class_type())) {
    primitive = new GeomTristrips(Geom::UH_static);

  } else if (egg_prim->is_of_type(EggTriangleFan::get_class_type())) {
    primitive = new GeomTrifans(Geom::UH_static);

  } else if (egg_prim->is_of_type(EggLine::get_class_type())) {
    if (egg_prim->size() == 2) {
      primitive = new GeomLines(Geom::UH_static);
    } else {
      primitive = new GeomLinestrips(Geom::UH_static);
    }

  } else if (egg_prim->is_of_type(EggPoint::get_class_type())) {
    primitive = new GeomPoints(Geom::UH_static);

  } else if (egg_prim->is_of_type(EggPatch::get_class_type())) {
    int num_vertices = egg_prim->size();
    primitive = new GeomPatches(num_vertices, Geom::UH_static);
  }

  if (primitive == nullptr) {
    // Don't know how to make this kind of primitive.
    egg2pg_cat.warning()
      << "Ignoring " << egg_prim->get_type() << "\n";
    return;
  }

  if (render_state->_flat_shaded) {
    primitive->set_shade_model(GeomPrimitive::SM_flat_first_vertex);

  } else if (egg_prim->get_shading() == EggPrimitive::S_overall) {
    primitive->set_shade_model(GeomPrimitive::SM_uniform);

  } else {
    primitive->set_shade_model(GeomPrimitive::SM_smooth);
  }

  // Insert the primitive into the set, but if we already have a primitive of
  // that type, reset the pointer to that one instead.
  PrimitiveUnifier pu(primitive);
  std::pair<UniquePrimitives::iterator, bool> result =
    unique_primitives.insert(UniquePrimitives::value_type(pu, primitive));

  if (result.second) {
    // This was the first primitive of this type.  Store it.
    primitives.push_back(primitive);

    if (egg2pg_cat.is_debug()) {
      egg2pg_cat.debug()
        << "First primitive of type " << primitive->get_type()
        << ": " << primitive << "\n";
    }
  }

  GeomPrimitive *orig_prim = (*result.first).second;

  // Make sure we don't try to put more than egg_max_indices into any one
  // GeomPrimitive.
  if (orig_prim->get_num_vertices() + egg_prim->size() <= (unsigned int)egg_max_indices) {
    primitive = orig_prim;

  } else if (orig_prim != primitive) {
    // If the old primitive is full, keep the new primitive from now on.
    (*result.first).second = primitive;

    if (egg2pg_cat.is_debug()) {
      egg2pg_cat.debug()
        << "Next primitive of type " << primitive->get_type()
        << ": " << primitive << "\n";
    }
    primitives.push_back(primitive);
  }

  // Now add the vertices.
  EggPrimitive::const_iterator vi;
  for (vi = egg_prim->begin(); vi != egg_prim->end(); ++vi) {
    primitive->add_vertex((*vi)->get_index());
  }
  primitive->close_primitive();
}

/**
 * Defines the PortalNode from the first polygon found within this group.
 */
void EggLoader::
set_portal_polygon(EggGroup *egg_group, PortalNode *pnode) {
  pnode->clear_vertices();

  PT(EggPolygon) poly = find_first_polygon(egg_group);
  if (poly != nullptr) {
    LMatrix4d mat = poly->get_vertex_to_node();

    EggPolygon::const_iterator vi;
    for (vi = poly->begin(); vi != poly->end(); ++vi) {
      LVertexd vert = (*vi)->get_pos3() * mat;
      pnode->add_vertex(LCAST(PN_stdfloat, vert));
    }
  }
}

/**
 * Defines the OccluderNode from the first polygon found within this group.
 */
void EggLoader::
set_occluder_polygon(EggGroup *egg_group, OccluderNode *pnode) {
  PT(EggPolygon) poly = find_first_polygon(egg_group);
  if (poly != nullptr) {
    if (poly->size() != 4) {
      egg2pg_cat.error()
        << "Invalid number of vertices for " << egg_group->get_name() << "\n";
    } else {
      LMatrix4d mat = poly->get_vertex_to_node();

      LPoint3d v0 = (*poly)[0]->get_pos3() * mat;
      LPoint3d v1 = (*poly)[1]->get_pos3() * mat;
      LPoint3d v2 = (*poly)[2]->get_pos3() * mat;
      LPoint3d v3 = (*poly)[3]->get_pos3() * mat;
      pnode->set_vertices(LCAST(PN_stdfloat, v0),
                          LCAST(PN_stdfloat, v1),
                          LCAST(PN_stdfloat, v2),
                          LCAST(PN_stdfloat, v3));

      if (poly->get_bface_flag()) {
        pnode->set_double_sided(true);
      }
    }
  }
}

/**
 * Returns the first EggPolygon found at or below the indicated node.
 */
PT(EggPolygon) EggLoader::
find_first_polygon(EggGroup *egg_group) {
  // Does this group have any polygons?
  EggGroup::const_iterator ci;
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
      // Yes!  Return the polygon.
      return DCAST(EggPolygon, (*ci));
    }
  }

  // Well, the group had no polygons; look for a child group that does.
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggGroup::get_class_type())) {
      EggGroup *child_group = DCAST(EggGroup, *ci);
      PT(EggPolygon) found = find_first_polygon(child_group);
      if (found != nullptr) {
        return found;
      }
    }
  }

  // We got nothing.
  return nullptr;
}

/**
 * Creates a single generic Sphere corresponding to the polygons associated
 * with this group.  This sphere is used by make_collision_sphere and
 * Polylight sphere.  It could be used for other spheres.
 */
bool EggLoader::
make_sphere(EggGroup *egg_group, EggGroup::CollideFlags flags,
            LPoint3 &center, PN_stdfloat &radius, LColor &color) {
  EggGroup *geom_group = find_collision_geometry(egg_group, flags);
  if (geom_group != nullptr) {
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
      // egg2pg_cat.debug() << "make_sphere d_center: " << d_center << "\n";

      // And the furthest vertex determines the radius.
      double radius2 = 0.0;
      for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
        EggVertex *vtx = (*vi);
        LPoint3d p3 = vtx->get_pos3();
                LVector3d v = p3 - d_center;
        radius2 = max(radius2, v.length_squared());
      }

      center = LCAST(PN_stdfloat, d_center);
      radius = sqrtf(radius2);

      // egg2pg_cat.debug() << "make_sphere radius: " << radius << "\n";
      vi = vertices.begin();
      EggVertex *clr_vtx = (*vi);
      color = clr_vtx->get_color();
      return true;
    }
  }
  return false;
}

/**
 * Creates a single generic Box corresponding to the polygons associated with
 * this group.  This box is used by make_collision_box.
 */
bool EggLoader::
make_box(EggGroup *egg_group, EggGroup::CollideFlags flags,
         const LMatrix4 &xform, LPoint3 &min_p, LPoint3 &max_p) {
  EggGroup *geom_group = find_collision_geometry(egg_group, flags);
  if (geom_group != nullptr) {
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

    // Now find the minmax points
    pset<EggVertex *>::const_iterator vi;
    vi = vertices.begin();

    if (vi == vertices.end()) {
      // No vertices, no bounding box.
      min_p.set(0, 0, 0);
      max_p.set(0, 0, 0);
      return false;
    }

    EggVertex *vertex = (*vi);
    LPoint3 min_pd = LCAST(PN_stdfloat, vertex->get_pos3()) * xform;
    LPoint3 max_pd = min_pd;

    for (++vi; vi != vertices.end(); ++vi) {
      vertex = (*vi);
      LPoint3 pos = LCAST(PN_stdfloat, vertex->get_pos3()) * xform;
      min_pd.set(min(min_pd[0], pos[0]),
                 min(min_pd[1], pos[1]),
                 min(min_pd[2], pos[2]));
      max_pd.set(max(max_pd[0], pos[0]),
                 max(max_pd[1], pos[1]),
                 max(max_pd[2], pos[2]));
    }

    min_p = min_pd;
    max_p = max_pd;
    return (min_pd != max_pd);
  }
  return false;
}

/**
 * Creates a single generic Box corresponding to the polygons associated with
 * this group.  This box is used by make_collision_box.
 */
bool EggLoader::
make_box(EggGroup *egg_group, EggGroup::CollideFlags flags,
         LPoint3 &min_p, LPoint3 &max_p, LColor &color) {

  color.set(1.0, 1.0, 1.0, 1.0);
  return make_box(egg_group, flags, LMatrix4::ident_mat(), min_p, max_p);
}

/**
 * Creates CollisionSolids corresponding to the collision geometry indicated
 * at the given node and below.
 */
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

  case EggGroup::CST_box:
    make_collision_box(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_inv_sphere:
    make_collision_inv_sphere(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_tube:
    make_collision_capsule(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_floor_mesh:
    make_collision_floor_mesh(egg_group, cnode, start_group->get_collide_flags());
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
  } else {
    egg2pg_cat.warning()
      << "Using <Collide> without 'descend' is deprecated.  'descend' "
      << "will become the default in a future version of Panda3D.\n";
  }
}

/**
 * Creates a single CollisionPlane corresponding to the first polygon
 * associated with this group.
 */
void EggLoader::
make_collision_plane(EggGroup *egg_group, CollisionNode *cnode,
                     EggGroup::CollideFlags flags) {
  EggGroup *geom_group = find_collision_geometry(egg_group, flags);
  if (geom_group != nullptr) {
    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
        CollisionPlane *csplane =
          create_collision_plane(DCAST(EggPolygon, *ci), egg_group);
        if (csplane != nullptr) {
          apply_collision_flags(csplane, flags);
          csplane->xform(cnode->get_transform()->get_mat());
          cnode->add_solid(csplane);
          return;
        }
      } else if ((*ci)->is_of_type(EggCompositePrimitive::get_class_type())) {
        EggCompositePrimitive *comp = DCAST(EggCompositePrimitive, *ci);
        PT(EggGroup) temp_group = new EggGroup;
        if (comp->triangulate_into(temp_group)) {
          make_collision_plane(temp_group, cnode, flags);
          return;
        }
      }
    }
  }
}



/**
 * Creates a single CollisionPolygon corresponding to the first polygon
 * associated with this group.
 */
void EggLoader::
make_collision_floor_mesh(EggGroup *egg_group, CollisionNode *cnode,
                       EggGroup::CollideFlags flags) {

  EggGroup *geom_group = find_collision_geometry(egg_group, flags);


  if (geom_group != nullptr) {
    create_collision_floor_mesh(cnode, geom_group,flags);
  }
}

/**
 * Creates a single CollisionPolygon corresponding to the first polygon
 * associated with this group.
 */
void EggLoader::
make_collision_polygon(EggGroup *egg_group, CollisionNode *cnode,
                       EggGroup::CollideFlags flags) {

  EggGroup *geom_group = find_collision_geometry(egg_group, flags);
  if (geom_group != nullptr) {
    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
        create_collision_polygons(cnode, DCAST(EggPolygon, *ci),
                                  egg_group, flags);
      } else if ((*ci)->is_of_type(EggCompositePrimitive::get_class_type())) {
        EggCompositePrimitive *comp = DCAST(EggCompositePrimitive, *ci);
        PT(EggGroup) temp_group = new EggGroup;
        if (comp->triangulate_into(temp_group)) {
          make_collision_polygon(temp_group, cnode, flags);
          return;
        }
      }
    }
  }
}


/**
 * Creates a series of CollisionPolygons corresponding to the polygons
 * associated with this group.
 */
void EggLoader::
make_collision_polyset(EggGroup *egg_group, CollisionNode *cnode,
                       EggGroup::CollideFlags flags) {
  EggGroup *geom_group = find_collision_geometry(egg_group, flags);
  if (geom_group != nullptr) {
    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
        create_collision_polygons(cnode, DCAST(EggPolygon, *ci),
                                  egg_group, flags);
      } else if ((*ci)->is_of_type(EggCompositePrimitive::get_class_type())) {
        EggCompositePrimitive *comp = DCAST(EggCompositePrimitive, *ci);
        PT(EggGroup) temp_group = new EggGroup;
        if (comp->triangulate_into(temp_group)) {
          make_collision_polyset(temp_group, cnode, flags);
        }
      }
    }
  }
}

/**
 * Creates a single CollisionSphere corresponding to the polygons associated
 * with this group.
 */
void EggLoader::
make_collision_sphere(EggGroup *egg_group, CollisionNode *cnode,
                      EggGroup::CollideFlags flags) {
  LPoint3 center;
  PN_stdfloat radius;
  LColor dummycolor;
  if (make_sphere(egg_group, flags, center, radius, dummycolor)) {
    CollisionSphere *cssphere =
      new CollisionSphere(center, radius);
    apply_collision_flags(cssphere, flags);
    cssphere->xform(cnode->get_transform()->get_mat());
    cnode->add_solid(cssphere);
  }
}

/**
 * Creates a single CollisionBox corresponding to the polygons associated with
 * this group.
 */
void EggLoader::
make_collision_box(EggGroup *egg_group, CollisionNode *cnode,
                   EggGroup::CollideFlags flags) {
  LPoint3 min_p;
  LPoint3 max_p;
  CPT(TransformState) transform = cnode->get_transform();
  if (make_box(egg_group, flags, transform->get_mat(), min_p, max_p)) {
    CollisionBox *csbox =
      new CollisionBox(min_p, max_p);
    apply_collision_flags(csbox, flags);
    cnode->add_solid(csbox);
  }
}

/**
 * Creates a single CollisionInvSphere corresponding to the polygons
 * associated with this group.
 */
void EggLoader::
make_collision_inv_sphere(EggGroup *egg_group, CollisionNode *cnode,
                          EggGroup::CollideFlags flags) {
  LPoint3 center;
  PN_stdfloat radius;
  LColor dummycolor;
  if (make_sphere(egg_group, flags, center, radius, dummycolor)) {
    CollisionInvSphere *cssphere =
      new CollisionInvSphere(center, radius);
    apply_collision_flags(cssphere, flags);
    cssphere->xform(cnode->get_transform()->get_mat());
    cnode->add_solid(cssphere);
  }
}

/**
 * Creates a single CollisionCapsule corresponding to the polygons associated
 * with this group.
 */
void EggLoader::
make_collision_capsule(EggGroup *egg_group, CollisionNode *cnode,
                       EggGroup::CollideFlags flags) {
  EggGroup *geom_group = find_collision_geometry(egg_group, flags);
  if (geom_group != nullptr) {
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

    // Now store the 3-d values in a vector for convenient access (and also
    // determine the centroid).  We compute this in node space.
    size_t num_vertices = vertices.size();
    if (num_vertices != 0) {
      pvector<LPoint3d> vpos;
      vpos.reserve(num_vertices);

      LPoint3d center(0.0, 0.0, 0.0);
      pset<EggVertex *>::const_iterator vi;
      for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
        EggVertex *vtx = (*vi);
        const LPoint3d &pos = vtx->get_pos3();
        vpos.push_back(pos);
        center += pos;
      }
      center /= (double)num_vertices;

      // Now that we have the centroid, we have to try to figure out the
      // cylinder's major axis.  Start by finding a point farthest from the
      // centroid.
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

      // The point we have found above, far_a, must be one one of the endcaps.
      // Now find another point, far_b, that is the farthest from far_a.  This
      // will be a point on the other endcap.
      radius2 = 0.0;
      LPoint3d far_b = center;
      for (i = 0; i < num_vertices; i++) {
        double dist2 = (vpos[i] - far_a).length_squared();
        if (dist2 > radius2) {
          radius2 = dist2;
          far_b = vpos[i];
        }
      }

      // Now we have far_a and far_b, one point on each endcap.  However,
      // these points are not necessarily centered on the endcaps, so we
      // haven't figured out the cylinder's axis yet (the line between far_a
      // and far_b will probably pass through the cylinder at an angle).

      // So we still need to determine the full set of points in each endcap.
      // To do this, we pass back through the set of points, categorizing each
      // point into either "endcap a" or "endcap b". We also leave a hefty
      // chunk of points in the middle uncategorized; this helps prevent us
      // from getting a little bit lopsided with points near the middle that
      // may appear to be closer to the wrong endcap.
      LPoint3d cap_a_center(0.0, 0.0, 0.0);
      LPoint3d cap_b_center(0.0, 0.0, 0.0);
      int num_a = 0;
      int num_b = 0;

      // This is the threshold length; points farther away from the center
      // than this are deemed to be in one endcap or the other.
      double center_length = (far_a - far_b).length() / 4.0;
      double center_length2 = center_length * center_length;

      for (i = 0; i < num_vertices; i++) {
        double dist2 = (vpos[i] - center).length_squared();
        if (dist2 > center_length2) {
          // This point is farther away from the center than center_length;
          // therefore it belongs in an endcap.
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

        // If the axis is *almost* parallel with a major axis, assume it is
        // meant to be exactly parallel.
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

        // Transform all of the points so that the major axis is along the Y
        // axis, and the origin is the center.  This is very similar to the
        // CollisionCapsule's idea of its canonical orientation (although not
        // exactly the same, since it is centered on the origin instead of
        // having point_a on the origin).  It makes it easier to determine the
        // length and radius of the cylinder.
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

        // And with the radius, we can determine the length.  We need to know
        // the radius first because we want the round endcaps to enclose all
        // points.
        double min_y = 0.0;
        double max_y = 0.0;

        for (i = 0; i < num_vertices; i++) {
          LVector2d v(vpos[i][0], vpos[i][2]);
          double radius2 = v.length_squared();

          if (vpos[i][1] < min_y) {
            // Adjust the Y pos to account for the point's distance from the
            // axis.
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

        CollisionCapsule *cscapsule =
          new CollisionCapsule(LCAST(PN_stdfloat, point_a), LCAST(PN_stdfloat, point_b),
                            radius);
        apply_collision_flags(cscapsule, flags);
        cscapsule->xform(cnode->get_transform()->get_mat());
        cnode->add_solid(cscapsule);
      }
    }
  }
}

/**
 * Does funny stuff to the CollisionSolid as appropriate, based on the
 * settings of the given CollideFlags.
 */
void EggLoader::
apply_collision_flags(CollisionSolid *solid, EggGroup::CollideFlags flags) {
  if ((flags & EggGroup::CF_intangible) != 0) {
    solid->set_tangible(false);
  }
  if ((flags & EggGroup::CF_level) != 0) {
    solid->set_effective_normal(LVector3::up());
  }
}

/**
 * Looks for the node, at or below the indicated node, that contains the
 * associated collision geometry.
 */
EggGroup *EggLoader::
find_collision_geometry(EggGroup *egg_group, EggGroup::CollideFlags flags) {
  if ((flags & EggGroup::CF_descend) != 0) {
    // If we have the "descend" instruction, we'll get to it when we get to
    // it.  Don't worry about it now.
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

  // Well, the group had no polygons; look for a child group that has the same
  // collision type.
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggGroup::get_class_type())) {
      EggGroup *child_group = DCAST(EggGroup, *ci);
      if (child_group->get_cs_type() == egg_group->get_cs_type()) {
        return child_group;
      }
    }
  }

  // We got nothing.
  return nullptr;
}

/**
 * Creates a single CollisionPlane from the indicated EggPolygon.
 */
CollisionPlane *EggLoader::
create_collision_plane(EggPolygon *egg_poly, EggGroup *parent_group) {
  if (!egg_poly->cleanup()) {
    egg2pg_cat.info()
      << "Ignoring degenerate collision plane in " << parent_group->get_name()
      << "\n";
    return nullptr;
  }

  if (!egg_poly->is_planar()) {
    egg2pg_cat.warning()
      << "Non-planar polygon defining collision plane in "
      << parent_group->get_name()
      << "\n";
  }

  pvector<LVertex> vertices;
  if (!egg_poly->empty()) {
    EggPolygon::const_iterator vi;
    vi = egg_poly->begin();

    LVertexd vert = (*vi)->get_pos3();
    vertices.push_back(LCAST(PN_stdfloat, vert));

    LVertexd last_vert = vert;
    ++vi;
    while (vi != egg_poly->end()) {
      vert = (*vi)->get_pos3();
      if (!vert.almost_equal(last_vert)) {
        vertices.push_back(LCAST(PN_stdfloat, vert));
      }

      last_vert = vert;
      ++vi;
    }
  }

  if (vertices.size() < 3) {
    return nullptr;
  }
  LPlane plane(vertices[0], vertices[1], vertices[2]);
  return new CollisionPlane(plane);
}

/**
 * Creates one or more CollisionPolygons from the indicated EggPolygon, and
 * adds them to the indicated CollisionNode.
 */
void EggLoader::
create_collision_polygons(CollisionNode *cnode, EggPolygon *egg_poly,
                          EggGroup *parent_group,
                          EggGroup::CollideFlags flags) {

  PT(EggGroup) group = new EggGroup;

  if (!egg_poly->triangulate_into(group, false)) {
    egg2pg_cat.info()
      << "Ignoring degenerate collision polygon in "
      << parent_group->get_name()
      << "\n";
    return;
  }

  if (group->size() != 1) {
    egg2pg_cat.info()
      << "Triangulating concave or non-planar collision polygon in "
      << parent_group->get_name()
      << "\n";
  }

  EggGroup::iterator ci;
  for (ci = group->begin(); ci != group->end(); ++ci) {
    EggPolygon *poly = DCAST(EggPolygon, *ci);

    pvector<LVertex> vertices;
    if (!poly->empty()) {
      EggPolygon::const_iterator vi;
      vi = poly->begin();

      LVertexd vert = (*vi)->get_pos3();
      vertices.push_back(LCAST(PN_stdfloat, vert));

      LVertexd last_vert = vert;
      ++vi;
      while (vi != poly->end()) {
        vert = (*vi)->get_pos3();
        if (!vert.almost_equal(last_vert)) {
          vertices.push_back(LCAST(PN_stdfloat, vert));
        }

        last_vert = vert;
        ++vi;
      }
    }

    if (vertices.size() >= 3) {
      const LVertex *vertices_begin = &vertices[0];
      const LVertex *vertices_end = vertices_begin + vertices.size();
      PT(CollisionPolygon) cspoly =
        new CollisionPolygon(vertices_begin, vertices_end);
      if (cspoly->is_valid()) {
        apply_collision_flags(cspoly, flags);
        cspoly->xform(cnode->get_transform()->get_mat());
        cnode->add_solid(cspoly);
      }
    }
  }
}

/**
 * Creates a CollisionFloorMesh from the indicated EggPolygons, and adds it to
 * the indicated CollisionNode.
 */
void EggLoader::
create_collision_floor_mesh(CollisionNode *cnode,
                          EggGroup *parent_group,
                          EggGroup::CollideFlags flags) {

  PT(EggGroup) group = new EggGroup;
  EggVertexPool pool("floorMesh");
  pool.local_object();
  EggGroup::const_iterator egi;
  for (egi = parent_group->begin(); egi != parent_group->end(); ++egi) {
    if ((*egi)->is_of_type(EggPolygon::get_class_type())) {
      EggPolygon * poly = DCAST(EggPolygon, *egi);
      if (!poly->triangulate_into(group, false)) {
        egg2pg_cat.info()
          << "Ignoring degenerate collision polygon in "
          << parent_group->get_name()
          << "\n";
        return;
      }

    }
  }
  if(group->size() == 0) {
    egg2pg_cat.info()
      << "empty collision solid\n";
    return;
  }
  PT(CollisionFloorMesh) cm = new CollisionFloorMesh;
  pvector<CollisionFloorMesh::TriangleIndices> triangles;

  EggGroup::iterator ci;
  for (ci = group->begin(); ci != group->end(); ++ci) {
    EggPolygon *poly = DCAST(EggPolygon, *ci);
    if (poly->get_num_vertices() == 3) {
      CollisionFloorMesh::TriangleIndices tri;

      // generate a shared vertex triangle from the vertex pool
      tri.p1=pool.create_unique_vertex(*poly->get_vertex(0))->get_index();
      tri.p2=pool.create_unique_vertex(*poly->get_vertex(1))->get_index();
      tri.p3=pool.create_unique_vertex(*poly->get_vertex(2))->get_index();

      triangles.push_back(tri);
    } else if (poly->get_num_vertices() == 4) {
      // this is a case that really shouldn't happen, but appears to be
      // required -split up the quad int 2 tris.
      CollisionFloorMesh::TriangleIndices tri;
      CollisionFloorMesh::TriangleIndices tri2;

      // generate a shared vertex triangle from the vertex pool
      tri.p1=pool.create_unique_vertex(*poly->get_vertex(0))->get_index();
      tri.p2=pool.create_unique_vertex(*poly->get_vertex(1))->get_index();
      tri.p3=pool.create_unique_vertex(*poly->get_vertex(2))->get_index();

      triangles.push_back(tri);

      // generate a shared vertex triangle from the vertex pool
      tri2.p1=tri.p1;
      tri2.p2=tri.p3;
      tri2.p3=pool.create_unique_vertex(*poly->get_vertex(3))->get_index();


      triangles.push_back(tri2);
    }
  }

  // Now we have a set of triangles, and a pool
  PT(CollisionFloorMesh) csfloor = new CollisionFloorMesh;


  EggVertexPool::const_iterator vi;
  for (vi = pool.begin(); vi != pool.end(); vi++) {
    csfloor->add_vertex(LCAST(PN_stdfloat,(*vi)->get_pos3()));
  }

  pvector<CollisionFloorMesh::TriangleIndices>::iterator ti;

  for (ti = triangles.begin(); ti != triangles.end(); ti++) {
    CollisionFloorMesh::TriangleIndices triangle = *ti;
    csfloor->add_triangle(triangle.p1, triangle.p2, triangle.p3);
  }
  csfloor->xform(cnode->get_transform()->get_mat());
  cnode->add_solid(csfloor);
}


/**
 * Walks back over the tree and applies the DeferredNodeProperties that were
 * saved up along the way.
 */
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

/**
 * Walks the hierarchy and calls expand_object_types() on each node, to expand
 * all of the ObjectType definitions in the file at once.  Also prunes any
 * nodes that are flagged "backstage".
 *
 * The return value is true if this node should be kept, false if it should be
 * pruned.
 */
bool EggLoader::
expand_all_object_types(EggNode *egg_node) {
  if (egg_node->is_of_type(EggGroup::get_class_type())) {
    EggGroup *egg_group = DCAST(EggGroup, egg_node);

    if (egg_group->get_num_object_types() != 0) {
      pset<string> expanded;
      pvector<string> expanded_history;
      if (!expand_object_types(egg_group, expanded, expanded_history)) {
        return false;
      }
    }
  }

  // Now recurse on children, and we might prune children from this list as we
  // go.
  if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *egg_group_node = DCAST(EggGroupNode, egg_node);
    EggGroupNode::const_iterator ci;
    ci = egg_group_node->begin();
    while (ci != egg_group_node->end()) {
      EggGroupNode::const_iterator cnext = ci;
      ++cnext;

      if (!expand_all_object_types(*ci)) {
        // Prune this child.
        egg_group_node->erase(ci);
      }
      ci = cnext;
    }
  }

  return true;
}

/**
 * Recursively expands the group's ObjectType string(s). It's recursive
 * because an ObjectType string might itself expand to another ObjectType
 * string, which is allowed; but we don't want to get caught in a cycle.
 *
 * The return value is true if the object type is expanded and the node is
 * valid, or false if the node should be ignored (e.g.  ObjectType
 * "backstage").
 */
bool EggLoader::
expand_object_types(EggGroup *egg_group, const pset<string> &expanded,
                    const pvector<string> &expanded_history) {
  int num_object_types = egg_group->get_num_object_types();

  // First, copy out the object types so we can recursively modify the list.
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
    }
  }

  return true;
}

/**
 * Further implementation of expand_object_types().
 */
bool EggLoader::
do_expand_object_type(EggGroup *egg_group, const pset<string> &expanded,
                      const pvector<string> &expanded_history,
                      const string &object_type) {
  // Try to find the egg syntax that the given objecttype is shorthand for.
  // First, look in the config file.

  ConfigVariableString egg_object_type
    ("egg-object-type-" + downcase(object_type), "");
  string egg_syntax = egg_object_type;

  if (!egg_object_type.has_value()) {
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
      // Now we've parsed the object type syntax, which might have added more
      // object types.  Recurse if necessary.
      if (egg_group->get_num_object_types() != 0) {
        if (!expand_object_types(egg_group, expanded, expanded_history)) {
          return false;
        }
      }
    }
  }

  return true;
}

/**
 * Extracts the combine_mode from the given egg texture, and returns its
 * corresponding TextureStage value.
 */
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

/**
 * Extracts the combine_source from the given egg texture, and returns its
 * corresponding TextureStage value.
 */
TextureStage::CombineSource EggLoader::
get_combine_source(const EggTexture *egg_tex,
                   EggTexture::CombineChannel channel, int n) {
  switch (egg_tex->get_combine_source(channel, n)) {
  case EggTexture::CS_unspecified:
    // The default source if it is unspecified is based on the parameter
    // index.
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

  case EggTexture::CS_constant_color_scale:
    return TextureStage::CS_constant_color_scale;

  case EggTexture::CS_last_saved_result:
    return TextureStage::CS_last_saved_result;
  };

  return TextureStage::CS_undefined;
}

/**
 * Extracts the combine_operand from the given egg texture, and returns its
 * corresponding TextureStage value.
 */
TextureStage::CombineOperand EggLoader::
get_combine_operand(const EggTexture *egg_tex,
                    EggTexture::CombineChannel channel, int n) {
  switch (egg_tex->get_combine_operand(channel, n)) {
  case EggTexture::CO_unspecified:
    if (channel == EggTexture::CC_rgb) {
      // The default operand for RGB is src_color, except for the third
      // parameter, which defaults to src_alpha.
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

/**
 * Converts the EggGroup's BlendMode to the corresponding
 * ColorBlendAttrib::Mode value.
 */
ColorBlendAttrib::Mode EggLoader::
get_color_blend_mode(EggGroup::BlendMode mode) {
  switch (mode) {
  case EggGroup::BM_unspecified:
  case EggGroup::BM_none:
    return ColorBlendAttrib::M_none;
  case EggGroup::BM_add:
    return ColorBlendAttrib::M_add;
  case EggGroup::BM_subtract:
    return ColorBlendAttrib::M_subtract;
  case EggGroup::BM_inv_subtract:
    return ColorBlendAttrib::M_inv_subtract;
  case EggGroup::BM_min:
    return ColorBlendAttrib::M_min;
  case EggGroup::BM_max:
    return ColorBlendAttrib::M_max;
  }

  return ColorBlendAttrib::M_none;
}

/**
 * Converts the EggGroup's BlendOperand to the corresponding
 * ColorBlendAttrib::Operand value.
 */
ColorBlendAttrib::Operand EggLoader::
get_color_blend_operand(EggGroup::BlendOperand operand) {
  switch (operand) {
  case EggGroup::BO_zero:
    return ColorBlendAttrib::O_zero;
  case EggGroup::BO_unspecified:
  case EggGroup::BO_one:
    return ColorBlendAttrib::O_one;
  case EggGroup::BO_incoming_color:
    return ColorBlendAttrib::O_incoming_color;
  case EggGroup::BO_one_minus_incoming_color:
    return ColorBlendAttrib::O_one_minus_incoming_color;
  case EggGroup::BO_fbuffer_color:
    return ColorBlendAttrib::O_fbuffer_color;
  case EggGroup::BO_one_minus_fbuffer_color:
    return ColorBlendAttrib::O_one_minus_fbuffer_color;
  case EggGroup::BO_incoming_alpha:
    return ColorBlendAttrib::O_incoming_alpha;
  case EggGroup::BO_one_minus_incoming_alpha:
    return ColorBlendAttrib::O_one_minus_incoming_alpha;
  case EggGroup::BO_fbuffer_alpha:
    return ColorBlendAttrib::O_fbuffer_alpha;
  case EggGroup::BO_one_minus_fbuffer_alpha:
    return ColorBlendAttrib::O_one_minus_fbuffer_alpha;
  case EggGroup::BO_constant_color:
    return ColorBlendAttrib::O_constant_color;
  case EggGroup::BO_one_minus_constant_color:
    return ColorBlendAttrib::O_one_minus_constant_color;
  case EggGroup::BO_constant_alpha:
    return ColorBlendAttrib::O_constant_alpha;
  case EggGroup::BO_one_minus_constant_alpha:
    return ColorBlendAttrib::O_one_minus_constant_alpha;
  case EggGroup::BO_incoming_color_saturate:
    return ColorBlendAttrib::O_incoming_color_saturate;
  case EggGroup::BO_color_scale:
    return ColorBlendAttrib::O_color_scale;
  case EggGroup::BO_one_minus_color_scale:
    return ColorBlendAttrib::O_one_minus_color_scale;
  case EggGroup::BO_alpha_scale:
    return ColorBlendAttrib::O_alpha_scale;
  case EggGroup::BO_one_minus_alpha_scale:
    return ColorBlendAttrib::O_one_minus_alpha_scale;
  }

  return ColorBlendAttrib::O_zero;
}

/**
 *
 */
bool EggLoader::VertexPoolTransform::
operator < (const EggLoader::VertexPoolTransform &other) const {
  if (_vertex_pool != other._vertex_pool) {
    return _vertex_pool < other._vertex_pool;
  }
  int compare = _transform.compare_to(other._transform, 0.001);
  if (compare != 0) {
    return compare < 0;
  }

  if (_bake_in_uvs.size() != other._bake_in_uvs.size()) {
    return _bake_in_uvs.size() < other._bake_in_uvs.size();
  }

  BakeInUVs::const_iterator ai, bi;
  ai = _bake_in_uvs.begin();
  bi = other._bake_in_uvs.begin();
  while (ai != _bake_in_uvs.end()) {
    nassertr(bi != other._bake_in_uvs.end(), false);
    if ((*ai) != (*bi)) {
      return (*ai) < (*bi);
    }
    ++ai;
    ++bi;
  }
  nassertr(bi == other._bake_in_uvs.end(), false);

  return false;
}
