// Filename: mouseWatcherGroup.cxx
// Created by:  drose (02Jul01)
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

#include "mouseWatcherGroup.h"


TypeHandle MouseWatcherGroup::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MouseWatcherGroup::
~MouseWatcherGroup() {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::add_region
//       Access: Published
//  Description: Adds the indicated region to the set of regions in
//               the group.  Returns true if it was successfully
//               added, or false if it was already on the list.
////////////////////////////////////////////////////////////////////
bool MouseWatcherGroup::
add_region(MouseWatcherRegion *region) {
  //return _regions.insert(region).second;

  PT(MouseWatcherRegion) pt = region;

  // We will only bother to check for duplicates in the region list if
  // we are building opt 1 or 2.  The overhead for doing this may be
  // too high if we have many regions.
#ifdef _DEBUG
  // See if the region is in the set/vector already
  Regions::const_iterator ri = 
    find(_regions.begin(), _regions.end(), pt);
  if (ri != _regions.end()) {
    // Already in the set, return false
    return false;
  }
#endif

  // Not in the set, add it and return true
  _regions.push_back(pt);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::has_region
//       Access: Published
//  Description: Returns true if the indicated region has already been
//               added to the MouseWatcherGroup, false otherwise.
////////////////////////////////////////////////////////////////////
bool MouseWatcherGroup::
has_region(MouseWatcherRegion *region) const {
  // See if the region is in the set/vector
  PT(MouseWatcherRegion) pt = region;
  Regions::const_iterator ri = 
    find(_regions.begin(), _regions.end(), pt);
  if (ri != _regions.end()) {
    // Found it
    return true;
  }
  // Did not find the region 
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::remove_region
//       Access: Published
//  Description: Removes the indicated region from the group.
//               Returns true if it was successfully removed, or false
//               if it wasn't there in the first place.
////////////////////////////////////////////////////////////////////
bool MouseWatcherGroup::
remove_region(MouseWatcherRegion *region) {
  //return _regions.erase(region) != 0;

  // See if the region is in the set/vector
  PT(MouseWatcherRegion) pt = region;
  Regions::iterator ri = 
    find(_regions.begin(), _regions.end(), pt);
  if (ri != _regions.end()) {
    // Found it, now erase it
    _regions.erase(ri);
    return true;
  }

  // Did not find the region to erase
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::find_region
//       Access: Published
//  Description: Returns a pointer to the first region found with the
//               indicated name.  If multiple regions share the same
//               name, the one that is returned is indeterminate.
////////////////////////////////////////////////////////////////////
MouseWatcherRegion *MouseWatcherGroup::
find_region(const string &name) const {
  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);
    if (region->get_name() == name) {
      return region;
    }
  }

  return (MouseWatcherRegion *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::clear_regions
//       Access: Published
//  Description: Removes all the regions from the group.
////////////////////////////////////////////////////////////////////
void MouseWatcherGroup::
clear_regions() {
  _regions.clear();
}
