// Filename: qpcullTraverser.cxx
// Created by:  drose (23Feb02)
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

#include "qpcullTraverser.h"
#include "renderState.h"
#include "cullHandler.h"
#include "dcast.h"
#include "qpgeomNode.h"

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
qpCullTraverser::
qpCullTraverser() {
  _initial_state = RenderState::make_empty();
  _world_transform = DCAST(TransformAttrib, TransformAttrib::make_identity());
  _cull_handler = (CullHandler *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_initial_state
//       Access: Public
//  Description: Sets the initial RenderState at the top of the scene
//               graph we are traversing.  If this is not set, the
//               default is the empty state.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
set_initial_state(const RenderState *initial_state) {
  _initial_state = initial_state;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_world_transform
//       Access: Public
//  Description: Specifies the net inverse transform of the camera
//               viewing the scene.  Normally, this is the same as the
//               transform specified in the initial state, but it
//               might be different if we are culling a subgraph,
//               for instance.
//
//               This is used to evaluate camera-dependent attributes
//               like billboards and LOD nodes.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
set_world_transform(const TransformAttrib *world_transform) {
  _world_transform = world_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::set_cull_handler
//       Access: Public
//  Description: Specifies the object that will receive the culled
//               Geoms.  This must be called before traverse().
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
set_cull_handler(CullHandler *cull_handler) {
  _cull_handler = cull_handler;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::traverse
//       Access: Public
//  Description: Begins the traversal from the indicated node.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
traverse(PandaNode *root) {
  nassertv(_cull_handler != (CullHandler *)NULL);

  r_traverse(root, _initial_state, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpCullTraverser::r_traverse
//       Access: Private
//  Description: The recursive traversal implementation.
////////////////////////////////////////////////////////////////////
void qpCullTraverser::
r_traverse(PandaNode *node, const RenderState *state, int flags) {
  CPT(RenderState) next_state = state->compose(node->get_state());

  if (node->is_geom_node()) {
    qpGeomNode *geom_node;
    DCAST_INTO_V(geom_node, node);
    
    int num_geoms = geom_node->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      Geom *geom = geom_node->get_geom(i);
      CPT(RenderState) geom_state = 
        next_state->compose(geom_node->get_geom_state(i));
      _cull_handler->found_geom(geom, geom_state);
    }
  }

  // Now visit all the node's children.
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_traverse(cr.get_child(i), next_state, flags);
  }
}
