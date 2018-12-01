/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseWatcher.cxx
 * @author drose
 * @date 2002-03-12
 */

#include "mouseWatcher.h"
#include "config_tform.h"
#include "dataGraphTraverser.h"
#include "mouseWatcherParameter.h"
#include "mouseAndKeyboard.h"
#include "mouseData.h"
#include "buttonEventList.h"
#include "mouseButton.h"
#include "throw_event.h"
#include "eventParameter.h"
#include "dataNodeTransmit.h"
#include "transformState.h"
#include "displayRegion.h"
#include "stereoDisplayRegion.h"
#include "geomVertexWriter.h"
#include "geomLinestrips.h"
#include "geomPoints.h"
#include "dcast.h"
#include "indent.h"
#include "lightMutexHolder.h"
#include "nearly_zero.h"

#include <algorithm>

using std::string;

TypeHandle MouseWatcher::_type_handle;

/**
 *
 */
MouseWatcher::
MouseWatcher(const string &name) :
  DataNode(name)
{
  _pixel_xy_input = define_input("pixel_xy", EventStoreVec2::get_class_type());
  _pixel_size_input = define_input("pixel_size", EventStoreVec2::get_class_type());
  _xy_input = define_input("xy", EventStoreVec2::get_class_type());
  _button_events_input = define_input("button_events", ButtonEventList::get_class_type());
  _pointer_events_input = define_input("pointer_events", PointerEventList::get_class_type());

  _pixel_xy_output = define_output("pixel_xy", EventStoreVec2::get_class_type());
  _pixel_size_output = define_output("pixel_size", EventStoreVec2::get_class_type());
  _xy_output = define_output("xy", EventStoreVec2::get_class_type());
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());

  _pixel_xy = new EventStoreVec2(LPoint2(0.0f, 0.0f));
  _xy = new EventStoreVec2(LPoint2(0.0f, 0.0f));
  _pixel_size = new EventStoreVec2(LPoint2(0.0f, 0.0f));
  _button_events = new ButtonEventList;

  _has_mouse = false;
  _internal_suppress = 0;
  _preferred_region = nullptr;
  _preferred_button_down_region = nullptr;
  _button_down = false;
  _eh = nullptr;
  _display_region = nullptr;
  _button_down_display_region = nullptr;

  _frame.set(-1.0f, 1.0f, -1.0f, 1.0f);

  _inactivity_timeout = inactivity_timeout;
  _has_inactivity_timeout = !IS_NEARLY_ZERO(_inactivity_timeout);

  _num_trail_recent = 0;
  _trail_log_duration = 0.0;
  _trail_log = new PointerEventList();

  _inactivity_timeout_event = "inactivity_timeout";
  _last_activity = 0.0;
  _inactivity_state = IS_active;

  // When this flag is true, the mouse pointer is allowed to be "entered" into
  // multiple regions simultaneously; when false, it will only be "within"
  // multiple regions, but "entered" into the topmost of those.
  _enter_multiple = false;

  // When this flag is true, moving the pointer into a region is enough to
  // click it.  The click is simulated with mouse button one.
  _implicit_click = false;
}

/**
 *
 */
MouseWatcher::
~MouseWatcher() {
}

/**
 * Removes the indicated region from the group.  Returns true if it was
 * successfully removed, or false if it wasn't there in the first place.
 */
bool MouseWatcher::
remove_region(MouseWatcherRegion *region) {
  LightMutexHolder holder(_lock);

  remove_region_from(_current_regions, region);
  if (region == _preferred_region) {
    if (_preferred_region != nullptr) {
      exit_region(_preferred_region, MouseWatcherParameter());
    }
    _preferred_region = nullptr;
  }
  if (region == _preferred_button_down_region) {
    _preferred_button_down_region = nullptr;
  }

  return MouseWatcherBase::do_remove_region(region);
}

/**
 * Returns the preferred region the mouse is over.  In the case of overlapping
 * regions, the region with the largest sort order is preferred; if two
 * regions have the same sort order, then the smaller region is preferred.
 */
MouseWatcherRegion *MouseWatcher::
get_over_region(const LPoint2 &pos) const {
  LightMutexHolder holder(_lock);

  Regions regions;
  get_over_regions(regions, pos);
  return get_preferred_region(regions);
}

/**
 * Adds the indicated group of regions to the set of regions the MouseWatcher
 * will monitor each frame.
 *
 * Since the MouseWatcher itself inherits from MouseWatcherBase, this
 * operation is normally not necessary--you can simply add the Regions you
 * care about one at a time.  Adding a complete group is useful when you may
 * want to explicitly remove the regions as a group later.
 *
 * Returns true if the group was successfully added, or false if it was
 * already on the list.
 */
bool MouseWatcher::
add_group(MouseWatcherGroup *group) {
  LightMutexHolder holder(_lock);

  // See if the group is in the setvector already
  PT(MouseWatcherGroup) pt = group;
  Groups::const_iterator gi =
    find(_groups.begin(), _groups.end(), pt);
  if (gi != _groups.end()) {
    // Already in the set, return false
    return false;
  }

#ifndef NDEBUG
  if (!_show_regions_render2d.is_empty()) {
    group->show_regions(_show_regions_render2d, _show_regions_bin_name,
                        _show_regions_draw_order);
  }
#endif  // NDEBUG

  // Not in the set, add it and return true
  _groups.push_back(pt);
  return true;
}

/**
 * Removes the indicated group from the set of extra groups associated with
 * the MouseWatcher.  Returns true if successful, or false if the group was
 * already removed or was never added via add_group().
 */
bool MouseWatcher::
remove_group(MouseWatcherGroup *group) {
  LightMutexHolder holder(_lock);
  LightMutexHolder holder2(group->_lock);

  group->do_sort_regions();

  Regions only_a, only_b, both;
  intersect_regions(only_a, only_b, both,
                    _current_regions, group->_regions);
  set_current_regions(only_a);

  if (has_region_in(both, _preferred_region)) {
    if (_preferred_region != nullptr) {
      exit_region(_preferred_region, MouseWatcherParameter());
    }
    _preferred_region = nullptr;
  }
  if (has_region_in(both, _preferred_button_down_region)) {
    _preferred_button_down_region = nullptr;
  }

#ifndef NDEBUG
  if (!_show_regions_render2d.is_empty()) {
    group->do_hide_regions();
  }
#endif  // NDEBUG

  // See if the group is in the setvector
  PT(MouseWatcherGroup) pt = group;
  Groups::iterator gi =
    find(_groups.begin(), _groups.end(), pt);
  if (gi != _groups.end()) {
    // Found it, now erase it
    _groups.erase(gi);
    return true;
  }

  // Did not find the group to erase
  return false;
}

/**
 * Atomically removes old_group from the MouseWatcher, and replaces it with
 * new_group.  Presumably old_group and new_group might have some regions in
 * common; these are handled properly.
 *
 * If old_group is not already present, simply adds new_group and returns
 * false.  Otherwise, removes old_group and adds new_group, and then returns
 * true.
 */
bool MouseWatcher::
replace_group(MouseWatcherGroup *old_group, MouseWatcherGroup *new_group) {
  if (old_group == new_group) {
    // Trivial.
    return true;
  }

  LightMutexHolder holder(_lock);

  LightMutexHolder holder2(old_group->_lock);
  LightMutexHolder holder3(new_group->_lock);

  old_group->do_sort_regions();
  new_group->do_sort_regions();

#ifndef NDEBUG
  if (!_show_regions_render2d.is_empty()) {
    old_group->do_hide_regions();
    new_group->do_show_regions(_show_regions_render2d, _show_regions_bin_name,
                               _show_regions_draw_order);
  }
#endif  // NDEBUG

  // Figure out the list of regions that change
  Regions remove, add, keep;
  intersect_regions(remove, add, keep,
                    old_group->_regions, new_group->_regions);

  Regions new_current_regions;
  bool any_new_current_regions = false;

  // Remove the old regions
  if (!remove.empty()) {
    Regions only_a, only_b, both;
    intersect_regions(only_a, only_b, both,
                      _current_regions, remove);
    new_current_regions.swap(only_a);
    any_new_current_regions = true;

    if (has_region_in(both, _preferred_region)) {
      if (_preferred_region != nullptr) {
        exit_region(_preferred_region, MouseWatcherParameter());
      }
      _preferred_region = nullptr;
    }
    if (has_region_in(both, _preferred_button_down_region)) {
      _preferred_button_down_region = nullptr;
    }
  }

  // Don't add the new regions--we have no reason to believe these should
  // become current; some of them may not even be under the mouse.
  /*
  // And add the new regions
  if (!add.empty()) {
    Regions new_list;
    if (any_new_current_regions) {
      intersect_regions(new_list, new_list, new_list,
                        new_current_regions, add);
    } else {
      intersect_regions(new_list, new_list, new_list,
                        _current_regions, add);
    }
    new_current_regions.swap(new_list);
    any_new_current_regions = true;
  }
  */

  if (any_new_current_regions) {
    set_current_regions(new_current_regions);
  }

  // Add the new group, if it's not already there.
  PT(MouseWatcherGroup) pt = new_group;
  Groups::iterator gi =
    find(_groups.begin(), _groups.end(), pt);
  if (gi == _groups.end()) {
    _groups.push_back(new_group);
  }

#ifndef NDEBUG
  if (!_show_regions_render2d.is_empty()) {
    new_group->do_update_regions();
  }
#endif  // NDEBUG

  // Remove the old group, if it is already there.
  pt = old_group;
  gi = find(_groups.begin(), _groups.end(), pt);
  if (gi != _groups.end()) {
    // Found it, now erase it
    _groups.erase(gi);
    return true;
  }

  // Did not find the group to erase
  return false;
}

/**
 * Returns the number of separate groups added to the MouseWatcher via
 * add_group().
 */
int MouseWatcher::
get_num_groups() const {
  LightMutexHolder holder(_lock);
  return _groups.size();
}

/**
 * Returns the nth group added to the MouseWatcher via add_group().
 */
MouseWatcherGroup *MouseWatcher::
get_group(int n) const {
  LightMutexHolder holder(_lock);
  nassertr(n >= 0 && n < (int)_groups.size(), nullptr);
  return _groups[n];
}

/**
 * If the duration is nonzero, causes the MouseWatcher to log the mouse's
 * trail.  Events older than the specified duration are discarded.  If the
 * duration is zero, logging is disabled.
 */
void MouseWatcher::
set_trail_log_duration(double duration) {
  if (duration < 0.0) {
    duration = 0.0;
  }
  _trail_log_duration = duration;
  discard_excess_trail_log();
}

/**
 * Discards trail log events whose age exceed the desired log duration.  Keeps
 * one event that is beyond the specified age, because otherwise, it is not
 * always possible to determine where the mouse was for the full logging
 * duration.  Also, keeps a minimum of two events in the queue.  If the
 * duration is zero, this method discards all trail events.
 */
void MouseWatcher::
discard_excess_trail_log() {
  if (_trail_log_duration == 0.0) {
    _trail_log->clear();
  } else {
    if (_trail_log->get_num_events() > 2) {
      double old = ClockObject::get_global_clock()->get_frame_time() - _trail_log_duration;
      while ((_trail_log->get_num_events() > 2)&&
             (_trail_log->get_time(0) <= old)&&
             (_trail_log->get_time(1) <= old)) {
        _trail_log->pop_front();
      }
    }
  }
}

/**
 * Returns a GeomNode that represents the mouse trail.  The intent is that you
 * should reparent this GeomNode to Render2D, and then forget about it.  The
 * MouseWatcher will continually update the trail node.  There is only one
 * trail node, it does not create a new one each time you call get_trail_node.
 *
 * This is not a particularly beautiful way to render a mouse trail.  It is
 * intended more for debugging purposes than for finished applications.  Even
 * so, It is suggested that you might want to apply a line thickness and
 * antialias mode to the line --- doing so makes it look a lot better.
 */
PT(GeomNode) MouseWatcher::
get_trail_node() {
  if (_trail_node == nullptr) {
    _trail_node = new GeomNode("Mouse Trail Node");
    update_trail_node();
  }
  return _trail_node;
}

/**
 * If you have previously fetched the trail node using get_trail_node, then
 * the MouseWatcher is continually updating the trail node every frame.  Using
 * clear_trail_node causes the MouseWatcher to forget the trail node and stop
 * updating it.
 */
void MouseWatcher::
clear_trail_node() {
  _trail_node = nullptr;
}

/**
 * Causes the trail node to represent the mouse trail.
 */
void MouseWatcher::
update_trail_node() {
  if (_trail_node == nullptr) {
    return;
  }
  _trail_node->remove_all_geoms();

  if (_trail_log->get_num_events() < 2) {
    return;
  }

  PT(GeomVertexData) data = new GeomVertexData
    ("mouseTrailSegs", GeomVertexFormat::get_v3(), Geom::UH_static);

  GeomVertexWriter vertex(data, InternalName::get_vertex());

  PT(GeomLinestrips) lines = new GeomLinestrips(Geom::UH_static);

  double xscale = 2.0 / _pixel_size->get_value().get_x();
  double yscale = 2.0 / _pixel_size->get_value().get_y();

  for (int i=0; i<(int)_trail_log->get_num_events(); i++) {
    double x = (_trail_log->get_xpos(i) * xscale) - 1.0;
    double y = (_trail_log->get_ypos(i) * yscale) - 1.0;
    vertex.add_data3(LVecBase3(x,0.0,-y));
    lines->add_vertex(i);
  }
  lines->close_primitive();

  PT(Geom) l_geom = new Geom(data);
  l_geom->add_primitive(lines);
  _trail_node->add_geom(l_geom);
}

/**
 * Can be used in conjunction with the inactivity timeout to inform the
 * MouseWatcher that the user has just performed some action which proves
 * he/she is present.  It may be necessary to call this for external events,
 * such as joystick action, that the MouseWatcher might otherwise not know
 * about.  This will reset the current inactivity timer.  When the inactivity
 * timer reaches the length of time specified by set_inactivity_timeout(),
 * with no keyboard or mouse activity and no calls to note_activity(), then
 * any buttons held will be automatically released.
 */
void MouseWatcher::
note_activity() {
  _last_activity = ClockObject::get_global_clock()->get_frame_time();
  switch (_inactivity_state) {
  case IS_active:
    break;

  case IS_inactive:
    _inactivity_state = IS_inactive_to_active;
    break;

  case IS_active_to_inactive:
    _inactivity_state = IS_active;
    break;

  case IS_inactive_to_active:
    break;
  }
}


/**
 *
 */
void MouseWatcher::
output(std::ostream &out) const {
  LightMutexHolder holder(_lock);
  DataNode::output(out);

  if (!_sorted) {
    ((MouseWatcher *)this)->do_sort_regions();
  }

  size_t count = _regions.size();
  for (MouseWatcherGroup *group : _groups) {
    count += group->get_num_regions();
  }

  out << " (" << count << " regions)";
}

/**
 *
 */
void MouseWatcher::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "MouseWatcher " << get_name() << ":\n";
  MouseWatcherBase::write(out, indent_level + 2);

  LightMutexHolder holder(_lock);
  for (MouseWatcherGroup *group : _groups) {
    indent(out, indent_level + 2)
      << "Subgroup:\n";
    group->write(out, indent_level + 4);
  }
}

/**
 * Fills up the "regions" list with the set of regions that the indicated
 * point is over, sorted in order by pointer.  Assumes the lock is held.
 */
void MouseWatcher::
get_over_regions(MouseWatcher::Regions &regions, const LPoint2 &pos) const {
  nassertv(_lock.debug_is_locked());

  // Scale the mouse coordinates into the frame.
  PN_stdfloat mx = (pos[0] + 1.0f) * 0.5f * (_frame[1] - _frame[0]) + _frame[0];
  PN_stdfloat my = (pos[1] + 1.0f) * 0.5f * (_frame[3] - _frame[2]) + _frame[2];

  // pos[0] = 2.0f * (mx - _frame[0])  (_frame[1] - _frame[0]) - 1.0f; pos[1]
  // = 2.0f * (my - _frame[2])  (_frame[3] - _frame[2]) - 1.0f;

  // Ensure the vector is empty before we begin.
  regions.clear();

  // Make sure there are no duplicates in the regions vector.
  if (!_sorted) {
    ((MouseWatcher *)this)->do_sort_regions();
  }

  for (MouseWatcherRegion *region : _regions) {
    const LVecBase4 &frame = region->get_frame();

    if (region->get_active() &&
        mx >= frame[0] && mx <= frame[1] &&
        my >= frame[2] && my <= frame[3]) {

      regions.push_back(region);
    }
  }

  // Also check all of our sub-groups.
  for (MouseWatcherGroup *group : _groups) {
    group->sort_regions();

    for (MouseWatcherRegion *region : group->_regions) {
      const LVecBase4 &frame = region->get_frame();

      if (region->get_active() &&
          mx >= frame[0] && mx <= frame[1] &&
          my >= frame[2] && my <= frame[3]) {

        regions.push_back(region);
      }
    }
  }

  // Now sort the regions by pointer.  By convention, the Regions vectors are
  // always kept in order by pointer, so we can do easy linear comparison and
  // intersection operations.
  sort(regions.begin(), regions.end());
}

/**
 * Returns the innermost region of all the regions indicated in the given
 * vector (usually, the regions the mouse is over).  This is the "preferred"
 * region that gets some special treatment.  Assumes the lock is already held.
 */
MouseWatcherRegion *MouseWatcher::
get_preferred_region(const MouseWatcher::Regions &regions) {
  if (regions.empty()) {
    return nullptr;
  }

  Regions::const_iterator ri;
  ri = regions.begin();
  MouseWatcherRegion *preferred = *ri;
  ++ri;
  while (ri != regions.end()) {
    MouseWatcherRegion *region = *ri;

    if (*region < *preferred) {
      preferred = region;
    }
    ++ri;
  }

  return preferred;
}

/**
 * Changes the "current" regions--the one we consider the mouse to be over--to
 * the indicated list, and throws whatever events are appropriate because of
 * that.
 *
 * The list passed in is destroyed.  Assumes the lock is already held.
 */
void MouseWatcher::
set_current_regions(MouseWatcher::Regions &regions) {
  nassertv(_lock.debug_is_locked());

  // Set up a parameter for passing through any change events.
  MouseWatcherParameter param;
  param.set_modifier_buttons(_mods);
  param.set_mouse(_mouse);

  // Now do a standard sorted comparison between the two vectors.
  Regions::const_iterator new_ri = regions.begin();
  Regions::const_iterator old_ri = _current_regions.begin();

  // Queue up all the new regions so we can send the within patterns all at
  // once, after all of the without patterns have been thrown.
  std::vector<MouseWatcherRegion *> new_regions;

  bool any_changes = false;
  while (new_ri != regions.end() && old_ri != _current_regions.end()) {
    if ((*new_ri) < (*old_ri)) {
      // Here's a new region that we didn't have last frame.
      MouseWatcherRegion *new_region = (*new_ri);
      new_regions.push_back(new_region);
      any_changes = true;
      ++new_ri;

    } else if ((*old_ri) < (*new_ri)) {
      // Here's a region we don't have any more.
      MouseWatcherRegion *old_region = (*old_ri);
      without_region(old_region, param);
      any_changes = true;
      ++old_ri;

    } else {
      // Here's a region that hasn't changed.
      ++new_ri;
      ++old_ri;
    }
  }

  while (new_ri != regions.end()) {
    // Here's a new region that we didn't have last frame.
    MouseWatcherRegion *new_region = (*new_ri);
    new_regions.push_back(new_region);
    any_changes = true;
    ++new_ri;
  }

  while (old_ri != _current_regions.end()) {
    // Here's a region we don't have any more.
    MouseWatcherRegion *old_region = (*old_ri);
    without_region(old_region, param);
    any_changes = true;
    ++old_ri;
  }

  if (any_changes) {
    // Now that we've compared the two vectors, simply swap them to set the
    // new vector.
    _current_regions.swap(regions);

    // And don't forget to throw all of the new regions' "within" events.
    std::vector<MouseWatcherRegion *>::const_iterator ri;
    for (ri = new_regions.begin(); ri != new_regions.end(); ++ri) {
      MouseWatcherRegion *new_region = (*ri);
      within_region(new_region, param);
    }
  }

  if (!_enter_multiple) {
    // Determine which is the "preferred region", if any.  This is the topmost
    // region that the mouse cursor is over, and the one that we are
    // considered "entered" into.
    MouseWatcherRegion *new_preferred_region =
      get_preferred_region(_current_regions);

    if (_button_down && new_preferred_region != _preferred_button_down_region) {
      // If the button's being held down, we're only allowed to select the
      // preferred button down region.
      new_preferred_region = nullptr;
    }

    if (new_preferred_region != _preferred_region) {
      if (_preferred_region != nullptr) {
        exit_region(_preferred_region, param);
      }
      _preferred_region = new_preferred_region;
      if (_preferred_region != nullptr) {
        enter_region(_preferred_region, param);
      }
    }
  }
}

/**
 * Empties the set of current regions.  Assumes the lock is already held.
 */
void MouseWatcher::
clear_current_regions() {
  nassertv(_lock.debug_is_locked());

  if (!_current_regions.empty()) {
    // Set up a parameter for passing through any change events.
    MouseWatcherParameter param;
    param.set_modifier_buttons(_mods);
    param.set_mouse(_mouse);

    Regions::const_iterator old_ri = _current_regions.begin();

    while (old_ri != _current_regions.end()) {
      // Here's a region we don't have any more.
      MouseWatcherRegion *old_region = (*old_ri);
      old_region->exit_region(param);
      throw_event_pattern(_leave_pattern, old_region, ButtonHandle::none());
      ++old_ri;
    }

    _current_regions.clear();

    if (_preferred_region != nullptr) {
      _preferred_region->exit_region(param);
      throw_event_pattern(_leave_pattern, _preferred_region, ButtonHandle::none());
      _preferred_region = nullptr;
    }
  }
}

#ifndef NDEBUG
/**
 * The protected implementation of show_regions().  This assumes the lock is
 * already held.
 */
void MouseWatcher::
do_show_regions(const NodePath &render2d, const string &bin_name,
                int draw_order) {
  MouseWatcherBase::do_show_regions(render2d, bin_name, draw_order);
  _show_regions_render2d = render2d;
  _show_regions_bin_name = bin_name;
  _show_regions_draw_order = draw_order;

  for (MouseWatcherGroup *group : _groups) {
    group->show_regions(render2d, bin_name, draw_order);
  }
}
#endif  // NDEBUG

#ifndef NDEBUG
/**
 * The protected implementation of hide_regions().  This assumes the lock is
 * already held.
 */
void MouseWatcher::
do_hide_regions() {
  MouseWatcherBase::do_hide_regions();
  _show_regions_render2d = NodePath();
  _show_regions_bin_name = string();
  _show_regions_draw_order = 0;

  for (MouseWatcherGroup *group : _groups) {
    group->hide_regions();
  }
}
#endif  // NDEBUG

/**
 * Computes the list of regions that are in both regions_a and regions_b, as
 * well as the list of regions only in regions_a, and the list of regions only
 * in regions_b.  Any or all of the three output lists may be the same object,
 * but they must be different objects from both of the input lists.
 *
 * It is assumed that both vectors are already sorted in pointer order.  It is
 * also assumed that any relevant locks are already held.
 */
void MouseWatcher::
intersect_regions(MouseWatcher::Regions &only_a,
                  MouseWatcher::Regions &only_b,
                  MouseWatcher::Regions &both,
                  const MouseWatcher::Regions &regions_a,
                  const MouseWatcher::Regions &regions_b) {
  // Now do a standard sorted intersection between the two vectors.
  Regions::const_iterator a_ri = regions_a.begin();
  Regions::const_iterator b_ri = regions_b.begin();

  while (a_ri != regions_a.end() && b_ri != regions_b.end()) {
    if ((*a_ri) < (*b_ri)) {
      // Here's a region in a, not in b.
      only_a.push_back(*a_ri);
      ++a_ri;

    } else if ((*b_ri) < (*a_ri)) {
      // Here's a region in b, not in a.
      only_b.push_back(*b_ri);
      ++b_ri;

    } else {
      // Here's a region in both vectors.
      both.push_back(*a_ri);
      ++a_ri;
      ++b_ri;
    }
  }
}

/**
 * Removes the indicated region from the given vector.  Assumes the vector is
 * sorted in pointer order.  Returns true if removed, false if it wasn't
 * there.  Assumes any relevent locks are already held.
 */
bool MouseWatcher::
remove_region_from(MouseWatcher::Regions &regions,
                   MouseWatcherRegion *region) {
  PT(MouseWatcherRegion) ptr = region;
  Regions::iterator ri = lower_bound(regions.begin(), regions.end(), ptr);
  if (ri != regions.end() && (*ri) == ptr) {
    // The region is in the vector.  Remove it.
    regions.erase(ri);
    return true;
  }

  return false;
}

/**
 * Returns true if the indicated region is a member of the given sorted list,
 * false otherwise.
 */
bool MouseWatcher::
has_region_in(const MouseWatcher::Regions &regions,
              MouseWatcherRegion *region) {
  PT(MouseWatcherRegion) ptr = region;
  Regions::const_iterator ri = lower_bound(regions.begin(), regions.end(), ptr);
  return (ri != regions.end() && (*ri) == ptr);
}

/**
 * Throws an event associated with the indicated region, using the given
 * pattern.
 */
void MouseWatcher::
throw_event_pattern(const string &pattern, const MouseWatcherRegion *region,
                    const ButtonHandle &button) {
  if (pattern.empty()) {
    return;
  }
#ifndef NDEBUG
  if (region != nullptr) {
    region->test_ref_count_integrity();
  }
#endif

  string button_name;
  if (button != ButtonHandle::none()) {
    if (!_mods.has_button(button)) {
      // We only prepend modifier names for buttons which are not themselves
      // modifiers.
      button_name = _mods.get_prefix();
    }
    button_name += button.get_name();
  }

  string event;
  for (size_t p = 0; p < pattern.size(); ++p) {
    if (pattern[p] == '%') {
      string cmd = pattern.substr(p + 1, 1);
      p++;
      if (cmd == "r") {
        if (region != nullptr) {
          event += region->get_name();
        }

      } else if (cmd == "b") {
        event += button.get_name();

      } else {
        tform_cat.error()
          << "Invalid symbol in event_pattern: %" << cmd << "\n";
      }
    } else {
      event += pattern[p];
    }
  }

  if (!event.empty()) {
    throw_event(event, EventParameter(region), EventParameter(button_name));
    if (_eh != nullptr)
      throw_event_directly(*_eh, event, EventParameter(region),
                           EventParameter(button_name));
  }
}

/**
 * Records the indicated mouse or keyboard button as being moved from last
 * position.
 */
void MouseWatcher::
move() {
  nassertv(_lock.debug_is_locked());

  MouseWatcherParameter param;
  param.set_modifier_buttons(_mods);
  param.set_mouse(_mouse);

  if (_preferred_button_down_region != nullptr) {
    _preferred_button_down_region->move(param);
  }
}

/**
 * Records the indicated mouse or keyboard button as being depressed.
 */
void MouseWatcher::
press(ButtonHandle button, bool keyrepeat) {
  nassertv(_lock.debug_is_locked());

  MouseWatcherParameter param;
  param.set_button(button);
  param.set_keyrepeat(keyrepeat);
  param.set_modifier_buttons(_mods);
  param.set_mouse(_mouse);

  if (MouseButton::is_mouse_button(button)) {
    // Mouse buttons are inextricably linked to the mouse position.

    if (!_button_down) {
      _preferred_button_down_region = _preferred_region;
    }
    _button_down = true;

    if (_preferred_button_down_region != nullptr) {
      _preferred_button_down_region->press(param);
      if (keyrepeat) {
        throw_event_pattern(_button_repeat_pattern,
                            _preferred_button_down_region, button);
      } else {
        throw_event_pattern(_button_down_pattern,
                            _preferred_button_down_region, button);
      }
    }

  } else {
    // It's a keyboard button; therefore, send the event to every region that
    // wants keyboard buttons, regardless of the mouse position.
    if (_preferred_region != nullptr) {
      // Our current region, the one under the mouse, always get all the
      // keyboard events, even if it doesn't set its keyboard flag.
      _preferred_region->press(param);
      consider_keyboard_suppress(_preferred_region);
    }

    if ((_internal_suppress & MouseWatcherRegion::SF_other_button) == 0) {
      // All the other regions only get the keyboard events if they set their
      // global keyboard flag, *and* the current region does not suppress
      // keyboard buttons.
      param.set_outside(true);
      global_keyboard_press(param);
    }
  }
}

/**
 * Records the indicated mouse or keyboard button as being released.
 */
void MouseWatcher::
release(ButtonHandle button) {
  nassertv(_lock.debug_is_locked());

  MouseWatcherParameter param;
  param.set_button(button);
  param.set_modifier_buttons(_mods);
  param.set_mouse(_mouse);

  if (MouseButton::is_mouse_button(button)) {
    // Button up.  Send the up event associated with the region(s) we were
    // over when the button went down.

    // There is some danger of losing button-up events here.  If more than one
    // button goes down together, we won't detect both of the button-up events
    // properly.
    if (_preferred_button_down_region != nullptr) {
      param.set_outside(_preferred_button_down_region != _preferred_region);
      _preferred_button_down_region->release(param);
      throw_event_pattern(_button_up_pattern,
                          _preferred_button_down_region, button);
    }

    _button_down = false;
    _preferred_button_down_region = nullptr;

  } else {
    // It's a keyboard button; therefore, send the event to every region that
    // wants keyboard buttons, regardless of the mouse position.
    if (_preferred_region != nullptr) {
      _preferred_region->release(param);
    }

    param.set_outside(true);
    global_keyboard_release(param);
  }
}

/**
 * Records that the indicated keystroke has been generated.
 */
void MouseWatcher::
keystroke(int keycode) {
  nassertv(_lock.debug_is_locked());

  MouseWatcherParameter param;
  param.set_keycode(keycode);
  param.set_modifier_buttons(_mods);
  param.set_mouse(_mouse);

  // Make sure there are no duplicates in the regions vector.
  if (!_sorted) {
    ((MouseWatcher *)this)->do_sort_regions();
  }

  // Keystrokes go to all those regions that want keyboard events, regardless
  // of which is the "preferred" region (that is, without respect to the mouse
  // position).  However, we do set the outside flag according to whether the
  // given region is the preferred region or not.

  for (MouseWatcherRegion *region : _regions) {
    if (region->get_keyboard()) {
      param.set_outside(region != _preferred_region);
      region->keystroke(param);
      consider_keyboard_suppress(region);
    }
  }

  // Also check all of our sub-groups.
  for (MouseWatcherGroup *group : _groups) {
    group->sort_regions();

    for (MouseWatcherRegion *region : group->_regions) {
      if (region->get_keyboard()) {
        param.set_outside(region != _preferred_region);
        region->keystroke(param);
        consider_keyboard_suppress(region);
      }
    }
  }
}

/**
 * Records that the indicated candidate string has been highlighted in the
 * IME.
 */
void MouseWatcher::
candidate(const std::wstring &candidate_string, size_t highlight_start,
          size_t highlight_end, size_t cursor_pos) {
  nassertv(_lock.debug_is_locked());

  MouseWatcherParameter param;
  param.set_candidate(candidate_string, highlight_start, highlight_end, cursor_pos);
  param.set_modifier_buttons(_mods);
  param.set_mouse(_mouse);

  // Make sure there are no duplicates in the regions vector.
  if (!_sorted) {
    ((MouseWatcher *)this)->do_sort_regions();
  }

  // Candidate strings go to all those regions that want keyboard events,
  // exactly like keystrokes, above.

  for (MouseWatcherRegion *region : _regions) {
    if (region->get_keyboard()) {
      param.set_outside(region != _preferred_region);
      region->candidate(param);
    }
  }

  // Also check all of our sub-groups.
  for (MouseWatcherGroup *group : _groups) {
    group->sort_regions();

    for (MouseWatcherRegion *region : group->_regions) {
      if (region->get_keyboard()) {
        param.set_outside(region != _preferred_region);
        region->candidate(param);
      }
    }
  }
}

/**
 * Calls press() on all regions that are interested in receiving global
 * keyboard events, except for the current region (which already received this
 * one).
 */
void MouseWatcher::
global_keyboard_press(const MouseWatcherParameter &param) {
  nassertv(_lock.debug_is_locked());

  // Make sure there are no duplicates in the regions vector.
  if (!_sorted) {
    ((MouseWatcher *)this)->do_sort_regions();
  }

  for (MouseWatcherRegion *region : _regions) {
    if (region != _preferred_region && region->get_keyboard()) {
      region->press(param);
      consider_keyboard_suppress(region);
    }
  }

  // Also check all of our sub-groups.
  for (MouseWatcherGroup *group : _groups) {
    group->sort_regions();

    for (MouseWatcherRegion *region : group->_regions) {
      if (region != _preferred_region && region->get_keyboard()) {
        region->press(param);
        consider_keyboard_suppress(region);
      }
    }
  }
}
/**
 * Calls release() on all regions that are interested in receiving global
 * keyboard events, except for the current region (which already received this
 * one).
 */
void MouseWatcher::
global_keyboard_release(const MouseWatcherParameter &param) {
  nassertv(_lock.debug_is_locked());

  // Make sure there are no duplicates in the regions vector.
  if (!_sorted) {
    ((MouseWatcher *)this)->do_sort_regions();
  }

  for (MouseWatcherRegion *region : _regions) {
    if (region != _preferred_region && region->get_keyboard()) {
      region->release(param);
    }
  }

  // Also check all of our sub-groups.
  for (MouseWatcherGroup *group : _groups) {
    group->sort_regions();

    for (MouseWatcherRegion *region : group->_regions) {
      if (region != _preferred_region && region->get_keyboard()) {
        region->release(param);
      }
    }
  }
}

/**
 * Called internally to indicate the mouse pointer is favoring the indicated
 * region.
 */
void MouseWatcher::
enter_region(MouseWatcherRegion *region, const MouseWatcherParameter &param) {
  nassertv(_lock.debug_is_locked());

  region->enter_region(param);
  throw_event_pattern(_enter_pattern, region, ButtonHandle::none());
  if (_implicit_click) {
    MouseWatcherParameter param1(param);
    param1.set_button(MouseButton::one());
    region->press(param1);
  }
}

/**
 * Called internally to indicate the mouse pointer is no longer favoring the
 * indicated region.
 */
void MouseWatcher::
exit_region(MouseWatcherRegion *region, const MouseWatcherParameter &param) {
  nassertv(_lock.debug_is_locked());

  if (_implicit_click) {
    MouseWatcherParameter param1(param);
    param1.set_button(MouseButton::one());
    region->release(param1);
  }
  region->exit_region(param);
  throw_event_pattern(_leave_pattern, region, ButtonHandle::none());
}

/**
 * Called from do_transmit_data() to indicate the mouse is not within the
 * window.
 */
void MouseWatcher::
set_no_mouse() {
  nassertv(_lock.debug_is_locked());

  if (_has_mouse) {
    // Hide the mouse pointer.
    if (!_geometry.is_null()) {
      _geometry->set_overall_hidden(true);
    }
  }

  _has_mouse = false;
  clear_current_regions();
}

/**
 * Called from do_transmit_data() to indicate the mouse is within the window,
 * and to specify its current position.
 */
void MouseWatcher::
set_mouse(const LVecBase2 &xy, const LVecBase2 &pixel_xy) {
  nassertv(_lock.debug_is_locked());

  if (!_geometry.is_null()) {
    // Transform the mouse pointer.
    _geometry->set_transform(TransformState::make_pos(LVecBase3(xy[0], 0, xy[1])));
    if (!_has_mouse) {
      // Show the mouse pointer.
      _geometry->set_overall_hidden(false);
    }
  }

  _has_mouse = true;
  _mouse = xy;
  _mouse_pixel = pixel_xy;

  Regions regions;
  get_over_regions(regions, _mouse);
  set_current_regions(regions);
}

/**
 * If we send any keyboard events to a region that has the SF_other_button
 * suppress flag set, that means we should not send the keyboard event along
 * the data graph.
 *
 * This method is called as each keyboard event is sent to a region; it should
 * update the internal _keyboard_suppress bitmask to indicate this.
 */
void MouseWatcher::
consider_keyboard_suppress(const MouseWatcherRegion *region) {
  if ((region->get_suppress_flags() & MouseWatcherRegion::SF_other_button) != 0) {
    _external_suppress |= MouseWatcherRegion::SF_other_button;
  }
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void MouseWatcher::
do_transmit_data(DataGraphTraverser *trav, const DataNodeTransmit &input,
                 DataNodeTransmit &output) {
  Thread *current_thread = trav->get_current_thread();
  LightMutexHolder holder(_lock);

  bool activity = false;

  // Initially, we do not suppress any events to objects below us in the data
  // graph.
  _internal_suppress = 0;
  _external_suppress = 0;

  // We always pass the pixel_size data through.
  EventStoreVec2 *pixel_size;
  DCAST_INTO_V(pixel_size, input.get_data(_pixel_size_input).get_ptr());
  output.set_data(_pixel_size_output, pixel_size);
  _pixel_size = pixel_size;

  if (input.has_data(_xy_input)) {
    // The mouse is within the window.  Get the current mouse position.
    const EventStoreVec2 *xy, *pixel_xy;
    DCAST_INTO_V(xy, input.get_data(_xy_input).get_ptr());
    DCAST_INTO_V(pixel_xy, input.get_data(_pixel_xy_input).get_ptr());

    LVecBase2 f = xy->get_value();
    LVecBase2 p = pixel_xy->get_value();

    // Asad: determine if mouse moved from last position
    const LVecBase2 &last_f = _xy->get_value();
    if (f != last_f) {
      activity = true;
      move();
    }

    if (_display_region != nullptr) {
      // If we've got a display region, constrain the mouse to it.
      if (constrain_display_region(_display_region, f, p, current_thread)) {
        set_mouse(f, p);

      } else {
        // The mouse is outside the display region, even though it's within
        // the window.  This is considered not having a mouse.
        set_no_mouse();

        // This also means we should suppress mouse button events below us.
        _internal_suppress |= MouseWatcherRegion::SF_mouse_button;
      }

    } else {
      // No display region; respect the whole window.
      set_mouse(f, p);
    }
  }

  // Code for recording the mouse trail.
  _num_trail_recent = 0;
  if (input.has_data(_pointer_events_input) && (_trail_log_duration > 0.0)) {
    const PointerEventList *this_pointer_events;
    DCAST_INTO_V(this_pointer_events, input.get_data(_pointer_events_input).get_ptr());
    _num_trail_recent = this_pointer_events->get_num_events();
    for (size_t i = 0; i < _num_trail_recent; i++) {
      bool in_win = this_pointer_events->get_in_window(i);
      int xpos = this_pointer_events->get_xpos(i);
      int ypos = this_pointer_events->get_ypos(i);
      int sequence = this_pointer_events->get_sequence(i);
      double time = this_pointer_events->get_time(i);
      _trail_log->add_event(in_win, xpos, ypos, sequence, time);
    }
  }
  if (_trail_log->get_num_events() > 0) {
    discard_excess_trail_log();
    update_trail_node();
  }
  if (_num_trail_recent > _trail_log->get_num_events()) {
    _num_trail_recent = _trail_log->get_num_events();
  }

  // If the mouse is over a particular region, or still considered owned by a
  // region because of a recent button-down event, that region determines
  // whether we suppress events below us.
  if (_preferred_region != nullptr) {
    _internal_suppress |= _preferred_region->get_suppress_flags();
  }

  ButtonEventList new_button_events;

  // Look for new button events.
  if (input.has_data(_button_events_input)) {
    const ButtonEventList *this_button_events;
    DCAST_INTO_V(this_button_events, input.get_data(_button_events_input).get_ptr());
    int num_events = this_button_events->get_num_events();
    for (int i = 0; i < num_events; i++) {
      const ButtonEvent &be = this_button_events->get_event(i);
      be.update_mods(_mods);

      switch (be._type) {
      case ButtonEvent::T_down:
        if (!_current_buttons_down.get_bit(be._button.get_index())) {
          // The button was not already depressed; thus, this is not
          // keyrepeat.
          activity = true;
          _current_buttons_down.set_bit(be._button.get_index());
          press(be._button, false);
          new_button_events.add_event(be);
          break;
        }
        // The button was already depressed, so this is really just keyrepeat.
        // Fall through.

      case ButtonEvent::T_repeat:
        _current_buttons_down.set_bit(be._button.get_index());
        press(be._button, true);
        new_button_events.add_event(ButtonEvent(be._button, ButtonEvent::T_repeat,
                                                be._time));
        break;

      case ButtonEvent::T_up:
        activity = true;
        _current_buttons_down.clear_bit(be._button.get_index());
        release(be._button);
        new_button_events.add_event(be);
        break;

      case ButtonEvent::T_keystroke:
        // We don't consider "keystroke" an activity event, because it might
        // be just keyrepeat.
        keystroke(be._keycode);
        new_button_events.add_event(be);
        break;

      case ButtonEvent::T_candidate:
        activity = true;
        candidate(be._candidate_string, be._highlight_start, be._highlight_end, be._cursor_pos);
        new_button_events.add_event(be);
        break;

      case ButtonEvent::T_resume_down:
        // _current_buttons_down.set_bit(be._button.get_index()); Don't call
        // press(), since the button wasn't actually pressed just now.
        new_button_events.add_event(be);
        break;

      case ButtonEvent::T_move:
        // This is handled below.
        break;

      case ButtonEvent::T_raw_down:
      case ButtonEvent::T_raw_up:
        // These are passed through.
        new_button_events.add_event(be);
        break;
      }
    }
  }

  if (!input.has_data(_xy_input)) {
    // No mouse in the window.  We check this down here, below the button
    // checking, in case the mouse left the window in the same frame it
    // released a button (particularly likely with a touchscreen input that's
    // emulating a mouse).
    set_no_mouse();
  }

  // Now check the inactivity timer.
  if (_has_inactivity_timeout) {
    if (activity) {
      note_activity();

    } else {
      double now = ClockObject::get_global_clock()->get_frame_time();
      double elapsed = now - _last_activity;

      // Toggle the inactivity state to inactive.
      if (elapsed > _inactivity_timeout) {
        switch (_inactivity_state) {
        case IS_active:
          _inactivity_state = IS_active_to_inactive;
          break;

        case IS_inactive:
          break;

        case IS_active_to_inactive:
          break;

        case IS_inactive_to_active:
          _inactivity_state = IS_inactive;
          break;
        }
      }
    }
  }

  switch (_inactivity_state) {
  case IS_active:
  case IS_inactive:
    break;

  case IS_active_to_inactive:
    // "Release" all of the currently-held buttons.
    if (tform_cat.is_debug()) {
      tform_cat.info()
        << "MouseWatcher detected " << _inactivity_timeout
        << " seconds of inactivity; releasing held buttons.\n";
    }
    {
      for (size_t i = 0; i < _current_buttons_down.get_num_bits(); ++i) {
        if (_current_buttons_down.get_bit(i)) {
          release(ButtonHandle((int)i));
          new_button_events.add_event(ButtonEvent(ButtonHandle((int)i), ButtonEvent::T_up));
        }
      }
    }
    _inactivity_state = IS_inactive;
    throw_event(_inactivity_timeout_event);
    break;

  case IS_inactive_to_active:
    // "Press" all of the buttons we "released" before.
    {
      for (size_t i = 0; i < _current_buttons_down.get_num_bits(); ++i) {
        if (_current_buttons_down.get_bit(i)) {
          press(ButtonHandle((int)i), false);
          new_button_events.add_event(ButtonEvent(ButtonHandle((int)i), ButtonEvent::T_down));
        }
      }
    }
    _inactivity_state = IS_active;
    break;
  }

  if (_has_mouse &&
      (_internal_suppress & MouseWatcherRegion::SF_mouse_position) == 0) {
    // Transmit the mouse position.
    _xy->set_value(_mouse);
    output.set_data(_xy_output, EventParameter(_xy));
    _pixel_xy->set_value(_mouse_pixel);
    output.set_data(_pixel_xy_output, EventParameter(_pixel_xy));
  }

  // Now transmit the buttons events down the graph.
  int suppress_buttons = ((_internal_suppress | _external_suppress) & MouseWatcherRegion::SF_any_button);

  _button_events->clear();

  int num_events = new_button_events.get_num_events();
  for (int i = 0; i < num_events; i++) {
    const ButtonEvent &be = new_button_events.get_event(i);
    bool suppress = true;

    if (be._type != ButtonEvent::T_keystroke &&
        MouseButton::is_mouse_button(be._button)) {
      suppress = ((suppress_buttons & MouseWatcherRegion::SF_mouse_button) != 0);
    } else {
      suppress = ((suppress_buttons & MouseWatcherRegion::SF_other_button) != 0);
    }

    if (!suppress || be._type == ButtonEvent::T_up) {
      // Don't suppress this button event; pass it through.
      _button_events->add_event(be);
    }
  }

  if (_button_events->get_num_events() != 0) {
    output.set_data(_button_events_output, EventParameter(_button_events));
  }
}

/**
 * Constrains the mouse coordinates to within the indicated DisplayRegion.  If
 * the mouse pointer does indeed fall within the DisplayRegion, rescales f and
 * p correspondingly, and returns true.  If the mouse pointer does not fall
 * within the DisplayRegion, leaves f and p unchanged, and returns false.
 */
bool MouseWatcher::
constrain_display_region(DisplayRegion *display_region,
                         LVecBase2 &f, LVecBase2 &p,
                         Thread *current_thread) {
  if (!_button_down) {
    _button_down_display_region = nullptr;
  }
  if (_button_down_display_region != nullptr) {
    // If the button went down over this DisplayRegion, we consider the button
    // within the same DisplayRegion until it is released (even if it wanders
    // outside the borders).
    display_region = _button_down_display_region;

  } else {
    // If it's a stereo DisplayRegion, we should actually call this method
    // twice, once for each eye, in case we have side-by-side stereo.
    if (display_region->is_stereo()) {
      StereoDisplayRegion *stereo_display_region;
      DCAST_INTO_R(stereo_display_region, display_region, false);
      return constrain_display_region(stereo_display_region->get_left_eye(), f, p, current_thread) ||
        constrain_display_region(stereo_display_region->get_right_eye(), f, p, current_thread);
    }
  }

  DisplayRegionPipelineReader dr_reader(display_region, current_thread);
  PN_stdfloat left, right, bottom, top;
  dr_reader.get_dimensions(left, right, bottom, top);

  // Need to translate this into DisplayRegion [0, 1] space
  PN_stdfloat x = (f[0] + 1.0f) / 2.0f;
  PN_stdfloat y = (f[1] + 1.0f) / 2.0f;

  if (_button_down_display_region == nullptr &&
      (x < left || x >= right || y < bottom || y >= top)) {
    // The mouse is outside the display region.
    return false;
  }

  // The mouse is within the display region; rescale it.
  if (_button_down) {
    _button_down_display_region = display_region;
  }

  // Scale in DR space
  PN_stdfloat xp = (x - left) / (right - left);
  // Translate back into [-1, 1] space
  PN_stdfloat xpp = (xp * 2.0f) - 1.0f;

  PN_stdfloat yp = (y - bottom) / (top - bottom);
  PN_stdfloat ypp = (yp * 2.0f) - 1.0f;

  int xo, yo, w, h;
  dr_reader.get_region_pixels_i(xo, yo, w, h);

  f.set(xpp, ypp);
  p.set(p[0] - xo, p[1] - yo);
  return true;
}
