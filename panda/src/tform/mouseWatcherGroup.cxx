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
#include "lineSegs.h"
#include "indent.h"

TypeHandle MouseWatcherGroup::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MouseWatcherGroup::
MouseWatcherGroup() {
#ifndef NDEBUG
  _show_regions = false;
  _color.set(0.4f, 0.6f, 1.0f, 1.0f);
#endif  // NDEBUG
}

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
//               the group.  It is an error to add the same region to
//               the set more than once.
////////////////////////////////////////////////////////////////////
void MouseWatcherGroup::
add_region(MouseWatcherRegion *region) {
  PT(MouseWatcherRegion) pt = region;

  // We will only bother to check for duplicates in the region list if
  // we are building a development Panda.  The overhead for doing this
  // may be too high if we have many regions.
#ifndef NDEBUG
  // See if the region is in the set/vector already
  Regions::const_iterator ri = 
    find(_regions.begin(), _regions.end(), pt);
  nassertv(ri == _regions.end());

  // Also add it to the vizzes if we have them.
  if (_show_regions) {
    nassertv(_vizzes.size() == _regions.size());
    _vizzes.push_back(make_viz_region(pt));
  }
#endif  // NDEBUG

  _regions.push_back(pt);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::has_region
//       Access: Published
//  Description: Returns true if the indicated region has already been
//               added to the MouseWatcherGroup, false otherwise.
////////////////////////////////////////////////////////////////////
bool MouseWatcherGroup::
has_region(MouseWatcherRegion *region) const {
  // See if the region is in the vector.
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
  // See if the region is in the vector.
  PT(MouseWatcherRegion) pt = region;
  Regions::iterator ri = 
    find(_regions.begin(), _regions.end(), pt);
  if (ri != _regions.end()) {
    // Found it, now erase it
#ifndef NDEBUG
    // Also remove it from the vizzes.
    if (_show_regions) {
      nassertr(_vizzes.size() == _regions.size(), false);
      size_t index = ri - _regions.begin();
      Vizzes::iterator vi = _vizzes.begin() + index;
      _show_regions_root.node()->remove_child(*vi);
      _vizzes.erase(vi);
    }
#endif  // NDEBUG    

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

#ifndef NDEBUG
  if (_show_regions) {
    _show_regions_root.node()->remove_all_children();
    _vizzes.clear();
  }
#endif  // NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::get_num_regions
//       Access: Published
//  Description: Returns the number of regions in the group.
////////////////////////////////////////////////////////////////////
int MouseWatcherGroup::
get_num_regions() const {
  return _regions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::get_region
//       Access: Published
//  Description: Returns the nth regions in the group.
////////////////////////////////////////////////////////////////////
MouseWatcherRegion *MouseWatcherGroup::
get_region(int n) const {
  nassertr(n >= 0 && n < (int)_regions.size(), NULL);
  return _regions[n];
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcherGroup::
output(ostream &out) const {
  out << "MouseWatcherGroup (" << _regions.size() << " regions)";
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcherGroup::
write(ostream &out, int indent_level) const {
  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);
    region->write(out, indent_level);
  }
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::show_regions
//       Access: Published
//  Description: Enables the visualization of all of the regions
//               handled by this MouseWatcherGroup.  The supplied
//               NodePath should be the root of the 2-d scene graph
//               for the window.
////////////////////////////////////////////////////////////////////
void MouseWatcherGroup::
show_regions(const NodePath &render2d) {
  _show_regions = true;
  _show_regions_root = render2d.attach_new_node("show_regions");
  _show_regions_root.set_bin("unsorted", 0);
  update_regions();
}
#endif  // NDEBUG

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::set_color
//       Access: Published
//  Description: Specifies the color used to draw the region
//               rectangles for the regions visualized by
//               show_regions().
////////////////////////////////////////////////////////////////////
void MouseWatcherGroup::
set_color(const Colorf &color) {
  _color = color;
  update_regions();
}
#endif  // NDEBUG

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::hide_regions
//       Access: Published
//  Description: Stops the visualization created by a previous call to
//               show_regions().
////////////////////////////////////////////////////////////////////
void MouseWatcherGroup::
hide_regions() {
  _show_regions_root.remove_node();
  _show_regions = false;
  _vizzes.clear();
}
#endif  // NDEBUG

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::update_regions
//       Access: Private
//  Description: Internally regenerates the show_regions()
//               visualization.
////////////////////////////////////////////////////////////////////
void MouseWatcherGroup::
update_regions() {
  _show_regions_root.node()->remove_all_children();
  _vizzes.clear();
  _vizzes.reserve(_regions.size());

  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    _vizzes.push_back(make_viz_region(*ri));
  }
}
#endif  // NDEBUG

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherGroup::make_viz_region
//       Access: Private
//  Description: Creates a node to represent the indicated region, and
//               attaches it to the _show_regions_root.  Does not add
//               it to _vizzes.
////////////////////////////////////////////////////////////////////
PandaNode *MouseWatcherGroup::
make_viz_region(MouseWatcherRegion *region) {
  LineSegs ls("show_regions");
  ls.set_color(_color);

  const LVecBase4f &f = region->get_frame();

  ls.move_to(LVector3f::rfu(f[0], 0.0f, f[2]));
  ls.draw_to(LVector3f::rfu(f[1], 0.0f, f[2]));
  ls.draw_to(LVector3f::rfu(f[1], 0.0f, f[3]));
  ls.draw_to(LVector3f::rfu(f[0], 0.0f, f[3]));
  ls.draw_to(LVector3f::rfu(f[0], 0.0f, f[2]));

  PT(PandaNode) node = ls.create();
  _show_regions_root.attach_new_node(node);

  return node;
}
#endif  // NDEBUG
