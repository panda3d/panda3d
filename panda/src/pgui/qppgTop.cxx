// Filename: qppgTop.cxx
// Created by:  drose (13Mar02)
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

#include "qppgTop.h"
#include "pgItem.h"
#include "pgMouseWatcherGroup.h"
#include "pgCullTraverser.h"
#include "cullBinAttrib.h"

#include "omniBoundingVolume.h"

TypeHandle qpPGTop::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpPGTop::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpPGTop::
qpPGTop(const string &name) : 
  PandaNode(name)
{
  _watcher_group = (PGMouseWatcherGroup *)NULL;

  // A qpPGTop node normally has an infinite bounding volume.  Screw
  // culling.
  set_bound(OmniBoundingVolume());
  set_final(true);

  // Also, screw state sorting.  By default, everything under PGTop
  // will be unsorted: rendered in scene graph order.  This is closer
  // to what the user wants anyway in a 2-d scene graph.

  // This override of 1000 should really be a system constant
  // somewhere.
  set_attrib(CullBinAttrib::make("unsorted", 0), 1000);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGTop::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpPGTop::
~qpPGTop() {
  set_mouse_watcher((qpMouseWatcher *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGTop::make_copy
//       Access: Protected, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *qpPGTop::
make_copy() const {
  return new qpPGTop(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGTop::has_cull_callback
//       Access: Protected, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool qpPGTop::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGTop::cull_callback
//       Access: Protected, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool qpPGTop::
cull_callback(qpCullTraverser *trav, CullTraverserData &data) {
  // Empty our set of regions in preparation for re-adding whichever
  // ones we encounter in the traversal that are current.
  clear_regions();

  // Now subsitute for the normal CullTraverser a special one of our
  // own choosing.  This just carries around a pointer back to the
  // PGTop node, for the convenience of PGItems to register themselves
  // as they are drawn.
  PGCullTraverser pg_trav(this, trav);
  pg_trav.traverse_below(data);

  // We've taken care of the traversal, thank you.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGTop::set_mouse_watcher
//       Access: Published
//  Description: Sets the MouseWatcher pointer that the qpPGTop object
//               registers its PG items with.  This must be set before
//               the PG items are active.
////////////////////////////////////////////////////////////////////
void qpPGTop::
set_mouse_watcher(qpMouseWatcher *watcher) {
  if (_watcher_group != (PGMouseWatcherGroup *)NULL) {
    _watcher_group->clear_top(this);
  }
  if (_watcher != (qpMouseWatcher *)NULL) {
    _watcher->remove_group(_watcher_group);
  }

  _watcher = watcher;
  _watcher_group = (PGMouseWatcherGroup *)NULL;

  if (_watcher != (qpMouseWatcher *)NULL) {
    // We create a new PGMouseWatcherGroup, but we don't own the
    // reference count; the watcher will own this for us.
    _watcher_group = new PGMouseWatcherGroup(this);
    _watcher->add_group(_watcher_group);
  }
}

/*
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
*/
