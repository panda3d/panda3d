// Filename: pgTop.cxx
// Created by:  drose (13Mar02)
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

#include "pgTop.h"
#include "pgMouseWatcherGroup.h"
#include "pgCullTraverser.h"
#include "cullBinAttrib.h"

#include "omniBoundingVolume.h"

TypeHandle PGTop::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGTop::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGTop::
PGTop(const string &name) : 
  PandaNode(name)
{
  _watcher_group = (PGMouseWatcherGroup *)NULL;

  // A PGTop node normally has an infinite bounding volume.  Screw
  // culling.
  set_bound(OmniBoundingVolume());
  set_final(true);

  // Also, screw state sorting.  By default, everything under PGTop
  // will be unsorted: rendered in scene graph order.  This is closer
  // to what the user wants anyway in a 2-d scene graph.

  set_attrib(CullBinAttrib::make("unsorted", 0));
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
//       Access: Protected, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *PGTop::
make_copy() const {
  return new PGTop(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::has_cull_callback
//       Access: Protected, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool PGTop::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::cull_callback
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
bool PGTop::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
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
