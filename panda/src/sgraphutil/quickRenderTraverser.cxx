// Filename: quickRenderTraverser.cxx
// Created by:  drose (24Jul01)
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

#include "quickRenderTraverser.h"
#include "config_sgraphutil.h"

#include "allAttributesWrapper.h"
#include "allTransitionsWrapper.h"
#include "graphicsStateGuardian.h"
#include "dftraverser.h"
#include "pruneTransition.h"
#include "geomNode.h"
#include "pStatTimer.h"
#include "wrt.h"

TypeHandle QuickRenderTraverser::_type_handle;

#ifndef CPPPARSER
PStatCollector QuickRenderTraverser::_draw_pcollector("Draw:Quick");
#endif

////////////////////////////////////////////////////////////////////
//     Function: QuickRenderTraverser::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
QuickRenderTraverser::
QuickRenderTraverser(GraphicsStateGuardian *gsg, TypeHandle graph_type,
              const ArcChain &arc_chain) :
  RenderTraverser(gsg, graph_type, arc_chain)
{
  _root = (Node *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: QuickRenderTraverser::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
QuickRenderTraverser::
~QuickRenderTraverser() {
}

////////////////////////////////////////////////////////////////////
//     Function: QuickRenderTraverser::traverse
//       Access: Public, Virtual
//  Description: This performs a normal, complete cull-and-render
//               traversal using this QuickRenderTraverser object.  State is
//               saved within the QuickRenderTraverser object so that the
//               next frame will be processed much more quickly than
//               the first frame.
////////////////////////////////////////////////////////////////////
void QuickRenderTraverser::
traverse(Node *root,
         const AllAttributesWrapper &initial_state,
         const AllTransitionsWrapper &net_trans) {
  // Statistics
  PStatTimer timer(_draw_pcollector);

  _root = root;
  _initial_state.apply_from(initial_state, net_trans);
  _arc_chain = ArcChain(_root);

  QuickRenderLevelState level_state;
  level_state._as_of = UpdateSeq::initial();
  level_state._under_instance = false;

  df_traverse(_root, *this,
              NullAttributeWrapper(), level_state, _graph_type);

  _root = (Node *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: QuickRenderTraverser::forward_arc
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool QuickRenderTraverser::
forward_arc(NodeRelation *arc, NullTransitionWrapper &,
            NullAttributeWrapper &, NullAttributeWrapper &,
            QuickRenderLevelState &level_state) {
  if (arc->has_transition(PruneTransition::get_class_type())) {
    return false;
  }

  Node *node = arc->get_child();

  if (node->get_num_parents(_graph_type) != 1) {
    /*
    const UpRelationPointers &urp = node->find_connection(_graph_type).get_up();
    int num_parents = urp.size();

    sgraphutil_cat.warning()
      << "Cannot support instancing via QuickRenderTraverser; "
      << *node << " has " << num_parents << " parents.\n"
      << "  parents are: ";
    nassertr(num_parents > 1, false);

    NodeRelation *parent_arc = urp[0];
    sgraphutil_cat.warning(false) << *parent_arc->get_parent();
    for (int i = 1; i < num_parents; i++) {
      parent_arc = urp[i];
      sgraphutil_cat.warning(false)
        << ", " << *parent_arc->get_parent();
    }
    sgraphutil_cat.warning(false) << "\n";

    return false;
    */
    level_state._under_instance = true;
  }

  if (implicit_app_traversal) {
    node->app_traverse(_arc_chain);
  }
  node->draw_traverse(_arc_chain);

  // We have to get a new _now timestamp, just in case either of the
  // above traversals changed it.
  UpdateSeq now = last_graph_update(_graph_type);

  UpdateSeq last_update = arc->get_last_update();
  if (level_state._as_of < last_update) {
    level_state._as_of = last_update;
  }

  mark_forward_arc(arc);
  _arc_chain.push_back(arc);
  
  _gsg->_nodes_pcollector.add_level(1);

  if (node->is_of_type(GeomNode::get_class_type())) {
    _gsg->_geom_nodes_pcollector.add_level(1);

    // Determine the net transition to this GeomNode, and render it.
    GeomNode *gnode = DCAST(GeomNode, node);
    AllTransitionsWrapper trans;

    if (level_state._under_instance) {
      // If we're under an instance node, we have to use the more
      // expensive wrt() operation.
      wrt(node, begin(), end(), (Node *)NULL, trans, _graph_type);

    } else {
      // Otherwise, we can use wrt_subtree() to get better caching.

      //Node *top_subtree = 
      wrt_subtree(arc, NULL,
                  level_state._as_of, now,
                  trans, _graph_type);

      /*
      cerr << "top_subtree is " << (void *)top_subtree;
      if (top_subtree != (Node *)NULL) {
        cerr << " is " << *top_subtree;
      }
      cerr << "\n";
      */
    }

    AllAttributesWrapper attrib;
    attrib.apply_from(_initial_state, trans);

    _gsg->set_state(attrib.get_attributes(), true);
    gnode->draw(_gsg);
  }

  return true;
}
