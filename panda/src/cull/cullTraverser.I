// Filename: cullTraverser.I
// Created by:  drose (13Apr00)
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

#include "cullStateSubtree.h"
#include "config_cull.h"

#include <directRenderTraverser.h>
#include <graphicsStateGuardian.h>

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::draw_geom
//       Access: Public
//  Description: Sets up the GSG to draw the indicated GeomNode in the
//               indicated state.  If the state so demands it and the
//               GeomNode has children, draws the rest of the
//               hierarchy below it immediately.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
draw_geom(GeomNode *geom_node, const AllTransitionsWrapper &initial_state) {
  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "Drawing " << *geom_node << " with state: " << initial_state << "\n";
  }
  nassertv(geom_node != (GeomNode *)NULL);
  _gsg->set_state(initial_state.get_transitions(), true);
  _gsg->prepare_display_region();
  geom_node->draw(_gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::draw_geom
//       Access: Public
//  Description: Sets up the GSG to draw the indicated GeomNode in the
//               indicated state.  If the state so demands it and the
//               GeomNode has children, draws the rest of the
//               hierarchy below it immediately.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
draw_geom(const ArcChain &arc_chain, const AllTransitionsWrapper &initial_state) {
  nassertv(!arc_chain.empty());
  GeomNode *geom_node;
  DCAST_INTO_V(geom_node, arc_chain.back()->get_child());
  draw_geom(geom_node, initial_state);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::draw_direct
//       Access: Public
//  Description: Draws the indicated subtree beginning at the
//               indicated node, and including all nested nodes below.
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
draw_direct(const ArcChain &arc_chain,
            const AllTransitionsWrapper &initial_state) {
  nassertv(!arc_chain.empty());
  Node *node = arc_chain.back()->get_child();
  if (cull_cat.is_spam()) {
    cull_cat.spam()
      << "Drawing " << *node << " in direct mode.\n";
  }
  nassertv(node != (Node *)NULL);
  DirectRenderTraverser drt(_gsg, _graph_type, arc_chain);
  drt.traverse(node, initial_state);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::add_instance
//       Access: Private
//  Description: This function should be called for each arc that
//               leads to a multiply-instanced node in the graph; that
//               is, each arc that points to a node with multiple
//               parents.  It will return a suitable CullStateLookup
//               pointer that should be passed to subsequent
//               add_geom_node() and add_instance() calls for nodes
//               that descend from this arc.
////////////////////////////////////////////////////////////////////
INLINE CullStateLookup *CullTraverser::
add_instance(NodeRelation *arc, const AllTransitionsWrapper &trans,
             Node *top_subtree, const CullLevelState &level_state) {
  return level_state._lookup->get_subtree
    (arc, trans, top_subtree, level_state._as_of);
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::find_bin_state
//       Access: Private
//  Description: Looks for a CullState in the set that corresponds to
//               the indicated set of transitions; if one is found,
//               returns its pointer; otherwise, creates a new one and
//               returns it.
////////////////////////////////////////////////////////////////////
INLINE CullState *CullTraverser::
find_bin_state(const AllTransitionsWrapper &trans) {
  PT(CullState) cs = new CullState(trans);

  // The insert operation will either succeed or fail, depending on
  // whether a matching CullState was already in the set.  It doesn't
  // matter; in either case, the return value represents the
  // corresponding CullState that is (now) stored in the set.
  pair<States::iterator, bool> result = _states.insert(cs);

#ifdef NOTIFY_DEBUG
  if (cull_cat.is_spam()) {
    if (result.second) {
      // The insert succeeded, so the CullState was not there
      // previously.
      cull_cat.spam()
        << "Created CullState " << (void *)cs << " for:\n";
      trans.write(cull_cat.spam(false), 2);
    } else {
      CullState *found_cs = *result.first;
      cull_cat.spam()
        << "Found existing CullState " << (void *)found_cs
        << " which has:\n";
      found_cs->get_transitions().write(cull_cat.spam(false), 2);
      cull_cat.spam(false)
        << "for:\n";
      trans.write(cull_cat.spam(false), 2);
    }
  }
#endif

  return *result.first;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverser::backward_arc
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CullTraverser::
backward_arc(NodeRelation *arc, NullTransitionWrapper &,
             NullTransitionWrapper &, NullTransitionWrapper &,
             const CullLevelState &) {
  mark_backward_arc(arc);
}
