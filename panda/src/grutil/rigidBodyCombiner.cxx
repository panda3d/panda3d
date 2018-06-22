/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rigidBodyCombiner.cxx
 * @author drose
 * @date 2007-02-22
 */

#include "rigidBodyCombiner.h"
#include "nodePath.h"
#include "geomNode.h"
#include "modelNode.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexAnimationSpec.h"
#include "sceneGraphReducer.h"
#include "omniBoundingVolume.h"
#include "cullTraverserData.h"

TypeHandle RigidBodyCombiner::_type_handle;


/**
 *
 */
RigidBodyCombiner::
RigidBodyCombiner(const std::string &name) : PandaNode(name) {
  set_cull_callback();

  _internal_root = new PandaNode(name);

  // We don't want to perform any additional culling once we get within the
  // RigidBodyCombiner.  The internal Geom's bounding volume is not updated
  // and might not be accurate.  However, the bounding volume of the
  // RigidBodyCombiner itself should be accurate, and this is sufficient.
  _internal_root->set_bounds(new OmniBoundingVolume);
  _internal_root->set_final(true);
}

/**
 *
 */
RigidBodyCombiner::
RigidBodyCombiner(const RigidBodyCombiner &copy) : PandaNode(copy) {
  set_cull_callback();

  _internal_root = copy._internal_root;
  _internal_transforms = copy._internal_transforms;
}

/**
 * Returns a newly-allocated PandaNode that is a shallow copy of this one.  It
 * will be a different pointer, but its internal data may or may not be shared
 * with that of the original PandaNode.  No children will be copied.
 */
PandaNode *RigidBodyCombiner::
make_copy() const {
  return new RigidBodyCombiner(*this);
}

/**
 * Walks through the entire subgraph of nodes rooted at this node, accumulates
 * all of the RenderAttribs and Geoms below this node, flattening them into
 * just one Geom (or as few as possible, if there are multiple different
 * states).
 *
 * Nodes that have transforms on them at the time of collect(), or any
 * ModelNodes with the preserve_transform flag, will be identified as "moving"
 * nodes, and their transforms will be monitored as they change in future
 * frames and each new transform directly applied to the vertices.
 *
 * This call must be made after adding any nodes to or removing any nodes from
 * the subgraph rooted at this node.  It should not be made too often, as it
 * is a relatively expensive call.  If you need to hide children of this node,
 * consider scaling them to zero (or very near zero), or moving them behind
 * the camera, instead.
 */
void RigidBodyCombiner::
collect() {
  _internal_root = new GeomNode(get_name());
  _internal_transforms.clear();
  _vd_table.clear();

  Children cr = get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_collect(cr.get_child(i), RenderState::make_empty(), nullptr);
  }

  _vd_table.clear();

  SceneGraphReducer gr;
  gr.apply_attribs(_internal_root);
  gr.collect_vertex_data(_internal_root, ~(SceneGraphReducer::CVD_format | SceneGraphReducer::CVD_name | SceneGraphReducer::CVD_animation_type));
  gr.unify(_internal_root, false);
}

/**
 * Returns a special NodePath that represents the internal node of this
 * object.  This is the node that is actually sent to the graphics card for
 * rendering; it contains the collection of the children of this node into as
 * few Geoms as possible.
 *
 * This node is filled up by the last call to collect().
 */
NodePath RigidBodyCombiner::
get_internal_scene() {
  return NodePath(_internal_root);
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool RigidBodyCombiner::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Pretend that all of our transforms have been modified (since we don't
  // really know which ones have).
  Thread *current_thread = Thread::get_current_thread();
  Transforms::iterator ti;
  for (ti = _internal_transforms.begin();
       ti != _internal_transforms.end();
       ++ti) {
    (*ti)->mark_modified(current_thread);
  }

  // Render the internal scene only--this is the optimized scene.
  CullTraverserData next_data(data, _internal_root);
  trav->traverse(next_data);

  // Do not directly render the nodes beneath this node.
  return false;
}

/**
 * Recursively visits each child or descedant of this node, accumulating state
 * and transform as we go.  When GeomNodes are encountered, their Geoms are
 * extracted and added to the _internal_root node.
 */
void RigidBodyCombiner::
r_collect(PandaNode *node, const RenderState *state,
          const VertexTransform *transform) {
  CPT(RenderState) next_state = state->compose(node->get_state());
  CPT(VertexTransform) next_transform = transform;
  if (!node->get_transform()->is_identity() ||
      (node->is_of_type(ModelNode::get_class_type()) &&
       DCAST(ModelNode, node)->get_preserve_transform() != ModelNode::PT_none)) {
    // This node has a transform we need to keep.
    PT(NodeVertexTransform) new_transform = new NodeVertexTransform(node, transform);
    _internal_transforms.push_back(new_transform);
    next_transform = new_transform.p();

  }

  if (node->is_geom_node()) {
    GeomNode *gnode = DCAST(GeomNode, node);
    GeomNode *root_gnode = DCAST(GeomNode, _internal_root);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; ++i) {
      PT(Geom) geom = gnode->get_geom(i)->make_copy();
      if (next_transform != nullptr) {
        geom->set_vertex_data(convert_vd(next_transform, geom->get_vertex_data()));
      }
      CPT(RenderState) gstate = next_state->compose(gnode->get_geom_state(i));
      root_gnode->add_geom(geom, gstate);
    }
  }

  Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_collect(cr.get_child(i), next_state, next_transform);
  }
}

/**
 * Converts a GeomVertexData to a new form in which all of the vertices are
 * transformed by the node's transform.
 */
PT(GeomVertexData) RigidBodyCombiner::
convert_vd(const VertexTransform *transform, const GeomVertexData *orig) {
  // First, unify this operation for unique transformdata combinations.  If we
  // encounter a given GeomVertexData more than once under the same transform,
  // we should return exactly the same GeomVertexData.
  VDTable::iterator vdti = _vd_table.find(VDUnifier(transform, orig));
  if (vdti != _vd_table.end()) {
    return (*vdti).second;
  }

  PT(GeomVertexFormat) format = new GeomVertexFormat(*orig->get_format());
  if (!orig->get_format()->has_column(InternalName::get_transform_blend())) {
    PT(GeomVertexArrayFormat) af = new GeomVertexArrayFormat();
    af->add_column(InternalName::get_transform_blend(), 1,
                   Geom::NT_uint16, Geom::C_index, 0, 2);
    format->add_array(af);
  }

  GeomVertexAnimationSpec spec;
  spec.set_panda();
  format->set_animation(spec);
  format->maybe_align_columns_for_animation();

  CPT(GeomVertexFormat) new_format = GeomVertexFormat::register_format(format);
  CPT(GeomVertexData) converted = orig->convert_to(new_format);
  PT(GeomVertexData) new_data = new GeomVertexData(*converted);

  if (new_data->get_transform_blend_table() == nullptr) {
    // Create a new table that has just the one blend: all vertices hard-
    // assigned to the indicated transform.
    PT(TransformBlendTable) new_table = new TransformBlendTable;
    new_table->add_blend(TransformBlend(transform, 1.0f));
    new_table->set_rows(SparseArray::range(0, new_data->get_num_rows()));
    new_data->set_transform_blend_table(new_table);
  } else {
    // The GeomVertexData already has a TransformBlendTable.  In this case,
    // we'll have to adjust it.  TODO.
  }

  // Store the result for the next time.
  _vd_table[VDUnifier(transform, orig)] = new_data;

  return new_data;
}
