/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseWatcherBase.cxx
 * @author rdb
 * @date 2014-01-13
 */

#include "mouseWatcherBase.h"
#include "lineSegs.h"
#include "indent.h"
#include "lightMutexHolder.h"

TypeHandle MouseWatcherBase::_type_handle;

/**
 *
 */
MouseWatcherBase::
MouseWatcherBase() :
  _lock("MouseWatcherBase")
{
  _sorted = true;
#ifndef NDEBUG
  _show_regions = false;
  _color.set(0.4, 0.6f, 1.0f, 1.0f);
#endif  // NDEBUG
}

/**
 *
 */
MouseWatcherBase::
~MouseWatcherBase() {
}

/**
 * Adds the indicated region to the set of regions in the group.  It is no
 * longer an error to call this for the same region more than once.
 */
void MouseWatcherBase::
add_region(PT(MouseWatcherRegion) region) {
  LightMutexHolder holder(_lock);

#ifndef NDEBUG
  // Also add it to the vizzes if we have them.
  if (UNLIKELY(_show_regions)) {
    // We need to check whether it is already in the set, so that we don't
    // create a duplicate viz.
    Regions::const_iterator ri =
      std::find(_regions.begin(), _regions.end(), region);

    if (ri == _regions.end()) {
      nassertv(_vizzes.size() == _regions.size());
      _vizzes.push_back(make_viz_region(region));
    } else {
      return;
    }
  }
#endif  // NDEBUG

  _regions.push_back(std::move(region));
  _sorted = false;
}

/**
 * Returns true if the indicated region has already been added to the
 * MouseWatcherBase, false otherwise.
 */
bool MouseWatcherBase::
has_region(MouseWatcherRegion *region) const {
  LightMutexHolder holder(_lock);

  PT(MouseWatcherRegion) ptr = region;

  if (_sorted) {
    // If the vector is already sorted, we can do this the quick way.
    Regions::const_iterator ri = lower_bound(_regions.begin(), _regions.end(), ptr);
    return (ri != _regions.end() && (*ri) == ptr);
  }

  // If the vector isn't sorted, do a linear scan.
  Regions::const_iterator ri = find(_regions.begin(), _regions.end(), ptr);
  return (ri != _regions.end());
}

/**
 * Removes the indicated region from the group.  Returns true if it was
 * successfully removed, or false if it wasn't there in the first place.
 */
bool MouseWatcherBase::
remove_region(MouseWatcherRegion *region) {
  LightMutexHolder holder(_lock);
  return do_remove_region(region);
}

/**
 * Returns a pointer to the first region found with the indicated name.  If
 * multiple regions share the same name, the one that is returned is
 * indeterminate.
 */
MouseWatcherRegion *MouseWatcherBase::
find_region(const std::string &name) const {
  LightMutexHolder holder(_lock);

  for (MouseWatcherRegion *region : _regions) {
    if (region->get_name() == name) {
      return region;
    }
  }

  return nullptr;
}

/**
 * Removes all the regions from the group.
 */
void MouseWatcherBase::
clear_regions() {
  LightMutexHolder holder(_lock);

  _regions.clear();
  _sorted = true;

#ifndef NDEBUG
  if (_show_regions) {
    _show_regions_root.node()->remove_all_children();
    _vizzes.clear();
  }
#endif  // NDEBUG
}

/**
 * Returns the number of regions in the group.
 */
size_t MouseWatcherBase::
get_num_regions() const {
  LightMutexHolder holder(_lock);
  if (!_sorted) {
    // Remove potential duplicates to get an accurate count.
    ((MouseWatcherBase *)this)->do_sort_regions();
  }
  return _regions.size();
}

/**
 * Returns the nth region of the group; returns NULL if there is no nth
 * region.  Note that this is not thread-safe; another thread might have
 * removed the nth region before you called this method.
 */
MouseWatcherRegion *MouseWatcherBase::
get_region(size_t n) const {
  LightMutexHolder holder(_lock);
  if (!_sorted) {
    ((MouseWatcherBase *)this)->do_sort_regions();
  }
  if (n < _regions.size()) {
    return _regions[n];
  }
  return nullptr;
}

/**
 *
 */
void MouseWatcherBase::
output(std::ostream &out) const {
  out << "MouseWatcherGroup (" << _regions.size() << " regions)";
}

/**
 *
 */
void MouseWatcherBase::
write(std::ostream &out, int indent_level) const {
  LightMutexHolder holder(_lock);

  for (MouseWatcherRegion *region : _regions) {
    region->write(out, indent_level);
  }
}

#if !defined(NDEBUG) || !defined(CPPPARSER)
/**
 * Enables the visualization of all of the regions handled by this
 * MouseWatcherBase.  The supplied NodePath should be the root of the 2-d
 * scene graph for the window.
 */
void MouseWatcherBase::
show_regions(const NodePath &render2d, const std::string &bin_name, int draw_order) {
#ifndef NDEBUG
  LightMutexHolder holder(_lock);
  do_show_regions(render2d, bin_name, draw_order);
#endif
}
#endif  // NDEBUG

#if !defined(NDEBUG) || !defined(CPPPARSER)
/**
 * Specifies the color used to draw the region rectangles for the regions
 * visualized by show_regions().
 */
void MouseWatcherBase::
set_color(const LColor &color) {
#ifndef NDEBUG
  LightMutexHolder holder(_lock);

  _color = color;
  do_update_regions();
#endif
}
#endif  // NDEBUG

#if !defined(NDEBUG) || !defined(CPPPARSER)
/**
 * Stops the visualization created by a previous call to show_regions().
 */
void MouseWatcherBase::
hide_regions() {
#ifndef NDEBUG
  LightMutexHolder holder(_lock);
  do_hide_regions();
#endif
}
#endif  // NDEBUG

#if !defined(NDEBUG) || !defined(CPPPARSER)
/**
 * Refreshes the visualization created by show_regions().
 */
void MouseWatcherBase::
update_regions() {
#ifndef NDEBUG
  LightMutexHolder holder(_lock);
  do_update_regions();
#endif
}
#endif  // NDEBUG


/**
 * Sorts all the regions in this group into pointer order.  Assumes the lock
 * is already held.
 */
void MouseWatcherBase::
do_sort_regions() {
  if (!_sorted) {
    _regions.sort_unique();
    _sorted = true;
  }
}

/**
 * The internal implementation of remove_region(); assumes the lock is already
 * held.
 */
bool MouseWatcherBase::
do_remove_region(MouseWatcherRegion *region) {
  // See if the region is in the vector.
  PT(MouseWatcherRegion) ptr = region;
  Regions::iterator ri;

  if (_sorted) {
    // Faster, binary search
    ri = lower_bound(_regions.begin(), _regions.end(), ptr);
  } else {
    // Unsorted, so use slower linear scan
    ri = find(_regions.begin(), _regions.end(), ptr);
  }

  if (ri != _regions.end() && (*ri) == ptr) {
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

#ifndef NDEBUG
/**
 * The protected implementation of show_regions().  This assumes the lock is
 * already held.
 */
void MouseWatcherBase::
do_show_regions(const NodePath &render2d, const std::string &bin_name,
                int draw_order) {
  do_hide_regions();
  _show_regions = true;
  _show_regions_root = render2d.attach_new_node("show_regions");
  _show_regions_root.set_bin(bin_name, draw_order);
  do_update_regions();
}
#endif  // NDEBUG

#ifndef NDEBUG
/**
 * The protected implementation of hide_regions().  This assumes the lock is
 * already held.
 */
void MouseWatcherBase::
do_hide_regions() {
  _show_regions_root.remove_node();
  _show_regions = false;
  _vizzes.clear();
}
#endif  // NDEBUG

#ifndef NDEBUG
/**
 * Internally regenerates the show_regions() visualization.  Assumes the lock
 * is already held.
 */
void MouseWatcherBase::
do_update_regions() {
  nassertv(_lock.debug_is_locked());

  if (_show_regions) {
    // Make sure we have no duplicates.
    do_sort_regions();

    _show_regions_root.node()->remove_all_children();
    _vizzes.clear();
    _vizzes.reserve(_regions.size());

    for (MouseWatcherRegion *region : _regions) {
      _vizzes.push_back(make_viz_region(region));
    }
  }
}
#endif  // NDEBUG


#ifndef NDEBUG
/**
 * Creates a node to represent the indicated region, and attaches it to the
 * _show_regions_root.  Does not add it to _vizzes.  Assumes the lock is
 * already held.
 */
PandaNode *MouseWatcherBase::
make_viz_region(MouseWatcherRegion *region) {
  nassertr(_lock.debug_is_locked(), nullptr);

  LineSegs ls("show_regions");
  ls.set_color(_color);

  const LVecBase4 &f = region->get_frame();

  ls.move_to(LVector3::rfu(f[0], 0.0f, f[2]));
  ls.draw_to(LVector3::rfu(f[1], 0.0f, f[2]));
  ls.draw_to(LVector3::rfu(f[1], 0.0f, f[3]));
  ls.draw_to(LVector3::rfu(f[0], 0.0f, f[3]));
  ls.draw_to(LVector3::rfu(f[0], 0.0f, f[2]));

  PT(PandaNode) node = ls.create();
  _show_regions_root.attach_new_node(node);

  return node;
}
#endif  // NDEBUG
