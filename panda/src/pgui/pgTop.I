// Filename: pgTop.I
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


////////////////////////////////////////////////////////////////////
//     Function: PGTop::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGTop::
PGTop(const PGTop &copy) :
  PandaNode(copy),
  _watcher(copy._watcher),
  _start_sort(copy._start_sort)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::get_mouse_watcher
//       Access: Published
//  Description: Returns the MouseWatcher pointer that the PGTop object
//               registers its PG items with, or NULL if the
//               MouseWatcher has not yet been set.
////////////////////////////////////////////////////////////////////
INLINE MouseWatcher *PGTop::
get_mouse_watcher() const {
  return _watcher;
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::set_start_sort
//       Access: Published
//  Description: Specifies the sort index that is assigned during the
//               traversal to the first PGItem that is discovered
//               during traversal.  Subsequent PGItems will be
//               assigned consecutively higher sort indexes.
//
//               This number is used by the MouseWatcher system to
//               rank the clickable mouse regions in the same order in
//               which the items are rendered, so that items on top
//               will receive mouse priority.
//
//               Normally, it makes the most sense to leave this
//               initial value at its default value of 0, unless you
//               need the PGItems to have a particular sort value with
//               respect to some other objects in the scene
//               (particularly with a second PGTop node).
////////////////////////////////////////////////////////////////////
INLINE void PGTop::
set_start_sort(int start_sort) {
  _start_sort = start_sort;
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::get_start_sort
//       Access: Published
//  Description: Returns the sort index that is assigned during the
//               traversal to the first PGItem that is discovered
//               during traversal.  See set_start_sort().
////////////////////////////////////////////////////////////////////
INLINE int PGTop::
get_start_sort() const {
  return _start_sort;
}


////////////////////////////////////////////////////////////////////
//     Function: PGTop::add_region
//       Access: Public
//  Description: Adds the indicated region to the set of regions in
//               the group.  Returns true if it was successfully
//               added, or false if it was already on the list.
////////////////////////////////////////////////////////////////////
INLINE bool PGTop::
add_region(MouseWatcherRegion *region) {
  if (_watcher_group == (PGMouseWatcherGroup *)NULL) {
    return false;
  }
  return _watcher_group->add_region(region);
}

////////////////////////////////////////////////////////////////////
//     Function: PGTop::clear_regions
//       Access: Public
//  Description: Removes all the regions from the group.
////////////////////////////////////////////////////////////////////
INLINE void PGTop::
clear_regions() {
  if (_watcher_group == (PGMouseWatcherGroup *)NULL) {
    return;
  }
  _watcher_group->clear_regions();
}
