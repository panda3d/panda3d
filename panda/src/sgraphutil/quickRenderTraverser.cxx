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
static PStatCollector _fooby_pcollector("Draw:Quick:Fooby");
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
traverse(Node *root, const AllTransitionsWrapper &initial_state) {
  // Statistics
  PStatTimer timer(_draw_pcollector);

  _root = root;
  _initial_state = initial_state;
  _arc_chain = ArcChain(_root);

  QuickRenderLevelState level_state;
  level_state._as_of = UpdateSeq::initial();
  level_state._under_instance = false;

  const DownRelationPointers &drp =
    _root->find_connection(_graph_type).get_down();

  // Now visit each of the children in turn.
  DownRelationPointers::const_iterator drpi;
  for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
    NodeRelation *arc = *drpi;
    r_traverse(DCAST(RenderRelation, arc), level_state);
  }

  _root = (Node *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: QuickRenderTraverser::r_traverse
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void QuickRenderTraverser::
r_traverse(RenderRelation *arc, const QuickRenderLevelState &level_state) {
  if (arc->has_transition(PruneTransition::get_class_type())) {
    return;
  }

  QuickRenderLevelState next_level_state(level_state);
  Node *node = arc->get_child();

  if (node->get_num_parents(_graph_type) != 1) {
    next_level_state._under_instance = true;
  }

  _arc_chain.push_back(arc);

  if (implicit_app_traversal) {
    node->app_traverse(_arc_chain);
  }
  node->draw_traverse(_arc_chain);

  // We have to get a new _now timestamp, just in case either of the
  // above traversals changed it.
  UpdateSeq now = last_graph_update(_graph_type);

  UpdateSeq last_update = arc->get_last_update();
  if (next_level_state._as_of < last_update) {
    next_level_state._as_of = last_update;
  }
  
  _gsg->_nodes_pcollector.add_level(1);

  if (node->is_of_type(GeomNode::get_class_type())) {
    _gsg->_geom_nodes_pcollector.add_level(1);

    // Determine the net transition to this GeomNode, and render it.
    GeomNode *gnode = DCAST(GeomNode, node);
    AllTransitionsWrapper trans;

    if (next_level_state._under_instance) {
      // If we're under an instance node, we have to use the more
      // expensive wrt() operation.
      wrt(node, begin(), end(), (Node *)NULL, trans, _graph_type);

    } else {
      // Otherwise, we can use wrt_subtree() to get better caching.
      wrt_subtree(arc, NULL,
                  next_level_state._as_of, now,
                  trans, _graph_type);
    }

    AllTransitionsWrapper attrib;
    attrib.compose_from(_initial_state, trans);

    _gsg->set_state(attrib);
    gnode->draw(_gsg);
  }

  const DownRelationPointers &drp =
    node->find_connection(_graph_type).get_down();

  // Now visit each of the children in turn.
  DownRelationPointers::const_iterator drpi;
  for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
    NodeRelation *arc = *drpi;
    r_traverse(DCAST(RenderRelation, arc), next_level_state);
  }

  _arc_chain.pop_back();
}
