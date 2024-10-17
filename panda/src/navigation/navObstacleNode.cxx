/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navObstacleNode.cxx
 * @author Maxwell175
 * @date 2022-12-15
 */


#include "navObstacleNode.h"
#include "omniBoundingVolume.h"
#include "cullTraverserData.h"
#include "cullTraverser.h"
#include <iostream>

TypeHandle NavObstacleNode::_type_handle;

/**
 * Initializes the NavObstacleNode with the specified name. Note that this does not add it to any NavMesh.
 * That must be done from a NavMesh object.
 */
NavObstacleNode::NavObstacleNode(const std::string &name) :
    PandaNode(name) {
  set_cull_callback();
  // NavObstacleNodes are hidden by default.
  set_overall_hidden(true);
  set_renderable();
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
bool NavObstacleNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Append our obstacle mesh vizzes to the drawing, even though they're not
  // actually part of the scene graph.
  PT(PandaNode) node = get_debug_geom();

  if(node != nullptr) {
    trav->traverse_down(data, node, data._net_transform, RenderState::make_empty());
  }

  // Now carry on to render our child nodes.
  return true;
}

/**
 * Called when needed to recompute the node's _internal_bound object.  Nodes
 * that contain anything of substance should redefine this to do the right
 * thing.
 */
void NavObstacleNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  internal_bounds = new OmniBoundingVolume;
  internal_vertices = 0;
}
