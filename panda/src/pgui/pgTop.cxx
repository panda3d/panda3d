// Filename: pgTop.cxx
// Created by:  drose (02Jul01)
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

#include "pgTop.h"
#include "pgItem.h"
#include "pgMouseWatcherGroup.h"

#include "arcChain.h"
#include "graphicsStateGuardian.h"
#include "config_sgraphutil.h"
#include "renderRelation.h"
#include "geomNode.h"
#include "allTransitionsWrapper.h"
#include "allTransitionsWrapper.h"
#include "wrt.h"
#include "switchNode.h"
#include "transformTransition.h"
#include "nodeTransitionWrapper.h"
#include "omniBoundingVolume.h"
#include "pruneTransition.h"

TypeHandle PGTop::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGTop::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGTop::
PGTop(const string &name) : NamedNode(name)
{
  _watcher_group = (PGMouseWatcherGroup *)NULL;
  _gsg = (GraphicsStateGuardian *)NULL;
  _trav = (RenderTraverser *)NULL;

  // A PGTop node normally has an infinite bounding volume.  Screw
  // culling.
  set_bound(OmniBoundingVolume());
  set_final(true);
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGTop::
~PGTop() {
  set_mouse_watcher((MouseWatcher *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *PGTop::
make_copy() const {
  return new PGTop(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::sub_render
//       Access: Public, Virtual
//  Description: Gets called during the draw traversal to render this
//               node and all nodes below it.  In the case of the
//               PGTop node, this uses a depth-first left-to-right
//               traversal to render all of the GeomNodes and PGItems
//               in the scene graph order.
////////////////////////////////////////////////////////////////////
bool PGTop::
sub_render(const AllTransitionsWrapper &input_trans,
           AllTransitionsWrapper &, RenderTraverser *trav) {
  _trans = input_trans;
  _gsg = trav->get_gsg();
  const ArcChain &chain = trav->get_arc_chain();

  // Empty our set of regions in preparation for re-adding whichever
  // ones we encounter in the traversal that are current.
  clear_regions();
  _sort_index = 0;

  // Start the traversal below the current node.
  const DownRelationPointers &drp = 
    find_connection(RenderRelation::get_class_type()).get_down();
  
  DownRelationPointers::const_iterator drpi;
  for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
    NodeRelation *arc = (*drpi);
    if (!arc->has_transition(PruneTransition::get_class_type())) {
      ArcChain next_chain(chain);
      next_chain.push_back(arc);
      r_traverse(arc->get_child(), next_chain);
    }
  }

  // We don't need the normal render traverser to do anything else;
  // we've done it all.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::has_sub_render
//       Access: Public, Virtual
//  Description: Should be redefined to return true if the function
//               sub_render(), above, expects to be called during
//               traversal.
////////////////////////////////////////////////////////////////////
bool PGTop::
has_sub_render() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::set_mouse_watcher
//       Access: Published
//  Description: Sets the MouseWatcher pointer that the PGTop object
//               registers its PG items with.  This must be set before
//               the PG items are active.
////////////////////////////////////////////////////////////////////
void PGTop::
set_mouse_watcher(MouseWatcher *watcher) {
  if (_watcher_group != (PGMouseWatcherGroup *)NULL) {
    _watcher_group->clear_top(this);
  }
  if (_watcher != (MouseWatcher *)NULL) {
    _watcher->remove_group(_watcher_group);
  }

  _watcher = watcher;
  _watcher_group = (PGMouseWatcherGroup *)NULL;

  if (_watcher != (MouseWatcher *)NULL) {
    // We create a new PGMouseWatcherGroup, but we don't own the
    // reference count; the watcher will own this for us.
    _watcher_group = new PGMouseWatcherGroup(this);
    _watcher->add_group(_watcher_group);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::r_traverse
//       Access: Private
//  Description: Handles the main recursive traversal of the scene
//               graph for nodes below the PGTop node.  This
//               implements a depth-first traversal.
////////////////////////////////////////////////////////////////////
void PGTop::
r_traverse(Node *node, const ArcChain &chain) {
  if (implicit_app_traversal) {
    node->app_traverse(chain);
  }
  node->draw_traverse(chain);
  _gsg->_nodes_pcollector.add_level(1);


  if (node->is_of_type(PGItem::get_class_type())) {
    PGItem *pgi = DCAST(PGItem, node);

    if (pgi->has_frame() && pgi->get_active()) {
      // The item has a frame, so we want to generate a region for it
      // and update the MouseWatcher.

      // Get the complete net transform to the PGItem from the top.
      LMatrix4f mat;
      
      NodeTransitionWrapper ntw(TransformTransition::get_class_type());
      wrt(pgi, chain.begin(), chain.end(), (Node *)NULL, 
          ntw, RenderRelation::get_class_type());
      const TransformTransition *tt;
      if (!get_transition_into(tt, ntw)) {
        // No relative transform.
        mat = LMatrix4f::ident_mat();
      } else {
        mat = tt->get_matrix();
      }
      
      // Now apply this transform to the item's frame.
      pgi->activate_region(this, mat, _sort_index);
      _sort_index++;

      add_region(pgi->get_region());
    }

    // And draw the item, however it wishes to be drawn.
    
    // Get the net transitions to the PGItem.
    AllTransitionsWrapper wrt_trans;
    wrt(pgi, chain.begin(), chain.end(), this,
        wrt_trans, RenderRelation::get_class_type());
    AllTransitionsWrapper complete_trans;
    complete_trans.compose_from(_trans, wrt_trans);
    pgi->draw_item(this, _gsg, complete_trans);

  } else if (node->is_of_type(GeomNode::get_class_type())) {
    _gsg->_geom_nodes_pcollector.add_level(1);
    GeomNode *geom = DCAST(GeomNode, node);

    // Get the complete state of the GeomNode.
    AllTransitionsWrapper wrt_trans;
    wrt(node, chain.begin(), chain.end(), this,
        wrt_trans, RenderRelation::get_class_type());
    AllTransitionsWrapper complete_trans;
    complete_trans.compose_from(_trans, wrt_trans);
    _gsg->set_state(complete_trans.get_transitions(), true);

    // Finally, draw the Geom.
    _gsg->prepare_display_region();
    geom->draw(_gsg);
  }


  // Continue the traversal.
  const DownRelationPointers &drp =
    node->find_connection(RenderRelation::get_class_type()).get_down();
  
  if (node->is_of_type(SwitchNode::get_class_type())) {
    SwitchNode *swnode = DCAST(SwitchNode, node);
    swnode->compute_switch(_trav);
    size_t i = 0;
    for (i = 0; i < drp.size(); i++) {
      if (swnode->is_child_visible(RenderRelation::get_class_type(), i)) {
        NodeRelation *arc = drp[i];
        if (!arc->has_transition(PruneTransition::get_class_type())) {
          ArcChain next_chain(chain);
          next_chain.push_back(arc);
          r_traverse(arc->get_child(), next_chain);
        }
      }
    }
  } else {
    DownRelationPointers::const_iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      NodeRelation *arc = (*drpi);
      if (!arc->has_transition(PruneTransition::get_class_type())) {
        ArcChain next_chain(chain);
        next_chain.push_back(arc);
        r_traverse(arc->get_child(), next_chain);
      }
    }
  }
}
