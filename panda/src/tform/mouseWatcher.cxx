// Filename: mouseWatcher.cxx
// Created by:  drose (13Jul00)
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

#include "mouseWatcher.h"
#include "config_tform.h"
#include "mouseWatcherParameter.h"

#include "mouse.h"
#include "mouseData.h"
#include "buttonEventDataTransition.h"
#include "mouseButton.h"
#include "throw_event.h"
#include "eventParameter.h"
#include "pruneTransition.h"
#include "transformTransition.h"

#include <algorithm>

TypeHandle MouseWatcher::_type_handle;

TypeHandle MouseWatcher::_xyz_type;
TypeHandle MouseWatcher::_pixel_xyz_type;
TypeHandle MouseWatcher::_button_events_type;

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
MouseWatcher::
MouseWatcher(const string &name) : DataNode(name) {
  _has_mouse = false;
  _suppress_flags = 0;
  _preferred_region = (MouseWatcherRegion *)NULL;
  _preferred_button_down_region = (MouseWatcherRegion *)NULL;
  _button_down = false;
  _eh = (EventHandler*)0L;

  // When this flag is true, the mouse pointer is allowed to be
  // "entered" into multiple regions simultaneously; when false, it
  // will only be "within" multiple regions, but "entered" into the
  // topmost of those.
  _enter_multiple = false;

  // When this flag is true, moving the pointer into a region is
  // enough to click it.  The click is simulated with mouse button
  // one.
  _implicit_click = false;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
MouseWatcher::
~MouseWatcher() {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::remove_region
//       Access: Published
//  Description: Removes the indicated region from the group.
//               Returns true if it was successfully removed, or false
//               if it wasn't there in the first place.
////////////////////////////////////////////////////////////////////
bool MouseWatcher::
remove_region(MouseWatcherRegion *region) {
  remove_region_from(_current_regions, region);
  if (region == _preferred_region) {
    _preferred_region = (MouseWatcherRegion *)NULL;
  }
  if (region == _preferred_button_down_region) {
    _preferred_button_down_region = (MouseWatcherRegion *)NULL;
  }

  return MouseWatcherGroup::remove_region(region);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_over_region
//       Access: Published
//  Description: Returns the preferred region the mouse is over.  In
//               the case of overlapping regions, the region with the
//               largest sort order is preferred; if two regions have
//               the same sort order, then the smaller region is
//               preferred.
////////////////////////////////////////////////////////////////////
MouseWatcherRegion *MouseWatcher::
get_over_region(const LPoint2f &pos) const {
  VRegions regions;
  get_over_regions(regions, pos);
  return get_preferred_region(regions);
}


////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcher::
output(ostream &out) const {
  DataNode::output(out);

  int count = _regions.size();
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    MouseWatcherGroup *group = (*gi);
    count += group->_regions.size();
  }

  out << " (" << count << " regions)";
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcher::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "MouseWatcher " << get_name() << ":\n";
  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);
    region->write(out, indent_level + 2);
  }

  if (!_groups.empty()) {
    Groups::const_iterator gi;
    for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
      MouseWatcherGroup *group = (*gi);
      indent(out, indent_level + 2)
        << "Subgroup:\n";
      for (ri = group->_regions.begin(); ri != group->_regions.end(); ++ri) {
        MouseWatcherRegion *region = (*ri);
        region->write(out, indent_level + 4);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::add_group
//       Access: Public
//  Description: Adds the indicated group of regions to the set of
//               regions the MouseWatcher will monitor each frame.
//
//               Since the MouseWatcher itself inherits from
//               MouseWatcherGroup, this operation is normally not
//               necessary--you can simply add the Regions you care
//               about one at a time.  Adding a complete group is
//               useful when you may want to explicitly remove the
//               regions as a group later.
//
//               Returns true if the group was successfully added, or
//               false if it was already on the list.
////////////////////////////////////////////////////////////////////
bool MouseWatcher::
add_group(PT(MouseWatcherGroup) group) {
  // return _groups.insert(group).second;

  // See if the group is in the set/vector already
  Groups::const_iterator gi = find(_groups.begin(), 
                                   _groups.end(), 
                                   group);
  if (gi != _groups.end()) {
    // Already in the set, return false
    return false;
  }

  // Not in the set, add it and return true
  _groups.push_back(group);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::remove_group
//       Access: Public
//  Description: Removes the indicated group from the set of extra
//               groups associated with the MouseWatcher.  Returns
//               true if successful, or false if the group was already
//               removed or was never added via add_group().
////////////////////////////////////////////////////////////////////
bool MouseWatcher::
remove_group(PT(MouseWatcherGroup) group) {
  remove_regions_from(_current_regions, group);
  if (group->has_region(_preferred_region)) {
    _preferred_region = (MouseWatcherRegion *)NULL;
  }
  if (group->has_region(_preferred_button_down_region)) {
    _preferred_button_down_region = (MouseWatcherRegion *)NULL;
  }

  // See if the group is in the set/vector
  Groups::iterator gi = find(_groups.begin(),
                            _groups.end(),
                            group);
  if (gi != _groups.end()) {
    // Found it, now erase it
    _groups.erase(gi);
    return true;
  }

  // Did not find the group to erase
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_over_regions
//       Access: Protected
//  Description: Fills up the "regions" list with the set of regions
//               that the indicated point is over, sorted in order by
//               pointer.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
get_over_regions(MouseWatcher::VRegions &regions, const LPoint2f &pos) const {
  // Ensure the vector is empty before we begin.
  regions.clear();

  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);
    const LVecBase4f &frame = region->get_frame();

    if (region->get_active() &&
        pos[0] >= frame[0] && pos[0] <= frame[1] &&
        pos[1] >= frame[2] && pos[1] <= frame[3]) {

      regions.push_back(region);
    }
  }

  // Also check all of our sub-groups.
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    MouseWatcherGroup *group = (*gi);
    for (ri = group->_regions.begin(); ri != group->_regions.end(); ++ri) {
      MouseWatcherRegion *region = (*ri);
      const LVecBase4f &frame = region->get_frame();
      
      if (region->get_active() &&
          pos[0] >= frame[0] && pos[0] <= frame[1] &&
          pos[1] >= frame[2] && pos[1] <= frame[3]) {
        
        regions.push_back(region);
      }
    }
  }

  // Now sort the regions by pointer.  By convention, the Regions
  // vectors are always kept in order by pointer, so we can do easy
  // linear comparison and intersection operations.
  sort(regions.begin(), regions.end());
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_preferred_region
//       Access: Protected, Static
//  Description: Returns the innermost region of all the regions
//               indicated in the given vector (usually, the regions
//               the mouse is over).  This is the "preferred" region
//               that gets some special treatment.
////////////////////////////////////////////////////////////////////
MouseWatcherRegion *MouseWatcher::
get_preferred_region(const MouseWatcher::VRegions &regions) {
  if (regions.empty()) {
    return (MouseWatcherRegion *)NULL;
  }

  VRegions::const_iterator ri;
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

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_current_regions
//       Access: Protected
//  Description: Changes the "current" regions--the one we consider the
//               mouse to be over--to the indicated list, and throws
//               whatever events are appropriate because of that.
//
//               The list passed in is destroyed.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
set_current_regions(MouseWatcher::VRegions &regions) {
  // Set up a parameter for passing through any change events.
  MouseWatcherParameter param;
  param.set_modifier_buttons(_mods);
  param.set_mouse(_mouse);

  // Now do a standard sorted comparison between the two vectors.
  VRegions::const_iterator new_ri = regions.begin();
  VRegions::const_iterator old_ri = _current_regions.begin();

  // Queue up all the new regions so we can send the within patterns
  // all at once, after all of the without patterns have been thrown.
  vector<MouseWatcherRegion *> new_regions;

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
    // Now that we've compared the two vectors, simply swap them to set
    // the new vector.
    _current_regions.swap(regions);

    // And don't forget to throw all of the new regions' "within" events.
    vector<MouseWatcherRegion *>::const_iterator ri;
    for (ri = new_regions.begin(); ri != new_regions.end(); ++ri) {
      MouseWatcherRegion *new_region = (*ri);
      within_region(new_region, param);
    }
  }

  if (!_enter_multiple) {
    // Determine which is the "preferred region", if any.  This is the
    // topmost region that the mouse cursor is over, and the one that
    // we are considered "entered" into.
    MouseWatcherRegion *new_preferred_region = 
      get_preferred_region(_current_regions);
    
    if (_button_down && new_preferred_region != _preferred_button_down_region) {
      // If the button's being held down, we're only allowed to select
      // the preferred button down region.
      new_preferred_region = (MouseWatcherRegion *)NULL;
    }

    if (new_preferred_region != _preferred_region) {
      if (_preferred_region != (MouseWatcherRegion *)NULL) {
        exit_region(_preferred_region, param);
      }
      _preferred_region = new_preferred_region;
      if (_preferred_region != (MouseWatcherRegion *)NULL) {
        enter_region(_preferred_region, param);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::clear_current_regions
//       Access: Protected
//  Description: Empties the set of current regions.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
clear_current_regions() {
  if (!_current_regions.empty()) {
    // Set up a parameter for passing through any change events.
    MouseWatcherParameter param;
    param.set_modifier_buttons(_mods);
    param.set_mouse(_mouse);
    
    VRegions::const_iterator old_ri = _current_regions.begin();
    
    while (old_ri != _current_regions.end()) {
      // Here's a region we don't have any more.
      MouseWatcherRegion *old_region = (*old_ri);
      old_region->exit(param);
      throw_event_pattern(_leave_pattern, old_region, ButtonHandle::none());
      ++old_ri;
    }
    
    _current_regions.clear();

    if (_preferred_region != (MouseWatcherRegion *)NULL) {
      _preferred_region->exit(param);
      throw_event_pattern(_leave_pattern, _preferred_region, ButtonHandle::none());
      _preferred_region = (MouseWatcherRegion *)NULL;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::intersect_regions
//       Access: Protected, Static
//  Description: Sets result to be the intersection of the list of
//               regions in regions_a and regions_b.  It is assumed
//               that both vectors are already sorted in pointer
//               order.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
intersect_regions(MouseWatcher::VRegions &result,
                  const MouseWatcher::VRegions &regions_a,
                  const MouseWatcher::VRegions &regions_b) {
  // Get a temporary vector for storing the result in.  We don't use
  // result directly, because it might be the same vector as one of a
  // or b.
  VRegions temp;

  // Now do a standard sorted intersection between the two vectors.
  VRegions::const_iterator a_ri = regions_a.begin();
  VRegions::const_iterator b_ri = regions_b.begin();

  while (a_ri != regions_a.end() && b_ri != regions_b.end()) {
    if ((*a_ri) < (*b_ri)) {
      // Here's a region in a, not in b.
      ++a_ri;

    } else if ((*b_ri) < (*a_ri)) {
      // Here's a region in b, not in a.
      ++b_ri;

    } else {
      // Here's a region in both vectors.
      temp.push_back(*a_ri);
      ++a_ri;
      ++b_ri;
    }
  }

  // Now store the result!
  result.swap(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::remove_region_from
//       Access: Protected, Static
//  Description: Removes the indicated region from the given vector.
//               Assumes the vector is sorted in pointer order.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
remove_region_from(MouseWatcher::VRegions &regions,
                   MouseWatcherRegion *region) {
  VRegions::iterator ri = 
    lower_bound(regions.begin(), regions.end(), region);
  if (ri != regions.end() && (*ri) == region) {
    // The region is in the vector.  Remove it.
    regions.erase(ri);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::remove_regions_from
//       Access: Protected, Static
//  Description: Removes all the regions in the indicated group from
//               the given vector.  Assumes the vector is sorted in
//               pointer order.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
remove_regions_from(MouseWatcher::VRegions &regions,
                    MouseWatcherGroup *group) {
  // Since the group stores a set of regions, which are also sorted in
  // pointer order, we can just do an intersection operation here.
  VRegions temp;

  VRegions::const_iterator a_ri = regions.begin();
  MouseWatcherGroup::Regions::const_iterator b_ri = group->_regions.begin();

  while (a_ri != regions.end() && b_ri != group->_regions.end()) {
    if ((*a_ri) < (*b_ri)) {
      // Here's a region in the group, not in regions.
      ++a_ri;

    } else if ((*b_ri) < (*a_ri)) {
      // Here's a region in regions, not in the group.
      temp.push_back(*b_ri);
      ++b_ri;

    } else {
      // Here's a region in the group and in regions.
      ++a_ri;
      ++b_ri;
    }
  }

  // Now store the result!
  regions.swap(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::throw_event_for
//       Access: Protected
//  Description: Throws an event associated with the indicated region,
//               using the given pattern.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
throw_event_pattern(const string &pattern, const MouseWatcherRegion *region,
                    const ButtonHandle &button) {
  if (pattern.empty()) {
    return;
  }
#ifndef NDEBUG
  if (region != (MouseWatcherRegion *)NULL) {
    region->test_ref_count_integrity();
  }
#endif

  string button_name;
  if (button != ButtonHandle::none()) {
    if (!_mods.has_button(button)) {
      // We only prepend modifier names for buttons which are not
      // themselves modifiers.
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
        if (region != (MouseWatcherRegion *)NULL) {
          event += button_name;
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
    if (_eh != (EventHandler*)0L)
      throw_event_directly(*_eh, event, EventParameter(region),
                           EventParameter(button_name));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::press
//       Access: Protected
//  Description: Records the indicated mouse or keyboard button as
//               being depressed.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
press(ButtonHandle button) {
  MouseWatcherParameter param;
  param.set_button(button);
  param.set_modifier_buttons(_mods);
  param.set_mouse(_mouse);

  if (MouseButton::is_mouse_button(button)) {
    // Mouse buttons are inextricably linked to the mouse position.
    
    if (!_button_down) {
      _preferred_button_down_region = _preferred_region;
    }
    _button_down = true;

    if (_preferred_button_down_region != (MouseWatcherRegion *)NULL) {
      _preferred_button_down_region->press(param);
      throw_event_pattern(_button_down_pattern,
                          _preferred_button_down_region, button);
    }
    
  } else {
    // It's a keyboard button; therefore, send the event to every
    // region that wants keyboard buttons, regardless of the mouse
    // position.
    if (_preferred_region != (MouseWatcherRegion *)NULL) {
      // Our current region, the one under the mouse, always get
      // all the keyboard events, even if it doesn't set its
      // keyboard flag.
      _preferred_region->press(param);
    }

    if ((_suppress_flags & MouseWatcherRegion::SF_other_button) == 0) {
      // All the other regions only get the keyboard events if they
      // set their global keyboard flag, *and* the current region does
      // not suppress keyboard buttons.
      param.set_outside(true);
      global_keyboard_press(param);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::release
//       Access: Protected
//  Description: Records the indicated mouse or keyboard button as
//               being released.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
release(ButtonHandle button) {
  MouseWatcherParameter param;
  param.set_button(button);
  param.set_modifier_buttons(_mods);
  param.set_mouse(_mouse);

  if (MouseButton::is_mouse_button(button)) {
    // Button up.  Send the up event associated with the region(s) we
    // were over when the button went down.
    
    // There is some danger of losing button-up events here.  If
    // more than one button goes down together, we won't detect
    // both of the button-up events properly.
    if (_preferred_button_down_region != (MouseWatcherRegion *)NULL) {
      param.set_outside(_preferred_button_down_region != _preferred_region);
      _preferred_button_down_region->release(param);
      throw_event_pattern(_button_up_pattern,
                          _preferred_button_down_region, button);
    }

    _button_down = false;
    _preferred_button_down_region = (MouseWatcherRegion *)NULL;
    
  } else {
    // It's a keyboard button; therefore, send the event to every
    // region that wants keyboard buttons, regardless of the mouse
    // position.
    if (_preferred_region != (MouseWatcherRegion *)NULL) {
      _preferred_region->release(param);
    }
    
    param.set_outside(true);
    global_keyboard_release(param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::global_keyboard_press
//       Access: Protected
//  Description: Calls press() on all regions that are interested in
//               receiving global keyboard events, except for the
//               current region (which already received this one).
////////////////////////////////////////////////////////////////////
void MouseWatcher::
global_keyboard_press(const MouseWatcherParameter &param) {
  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);

    if (region != _preferred_region && region->get_keyboard()) {
      region->press(param);
    }
  }

  // Also check all of our sub-groups.
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    MouseWatcherGroup *group = (*gi);
    for (ri = group->_regions.begin(); ri != group->_regions.end(); ++ri) {
      MouseWatcherRegion *region = (*ri);

      if (region != _preferred_region && region->get_keyboard()) {
        region->press(param);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::global_keyboard_release
//       Access: Protected
//  Description: Calls release() on all regions that are interested in
//               receiving global keyboard events, except for the
//               current region (which already received this one).
////////////////////////////////////////////////////////////////////
void MouseWatcher::
global_keyboard_release(const MouseWatcherParameter &param) {
  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);

    if (region != _preferred_region && region->get_keyboard()) {
      region->release(param);
    }
  }

  // Also check all of our sub-groups.
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    MouseWatcherGroup *group = (*gi);
    for (ri = group->_regions.begin(); ri != group->_regions.end(); ++ri) {
      MouseWatcherRegion *region = (*ri);

      if (region != _preferred_region && region->get_keyboard()) {
        region->release(param);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::enter_region
//       Access: Protected
//  Description: Called internally to indicate the mouse pointer is no
//               longer favoring the indicated region.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
enter_region(MouseWatcherRegion *region, const MouseWatcherParameter &param) {
  region->enter(param);
  throw_event_pattern(_enter_pattern, region, ButtonHandle::none());
  if (_implicit_click) {
    MouseWatcherParameter param1(param);
    param1.set_button(MouseButton::one());
    region->press(param1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::exit_region
//       Access: Protected
//  Description: Called internally to indicate the mouse pointer is no
//               longer favoring the indicated region.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
exit_region(MouseWatcherRegion *region, const MouseWatcherParameter &param) {
  if (_implicit_click) {
    MouseWatcherParameter param1(param);
    param1.set_button(MouseButton::one());
    region->release(param1);
  }
  region->exit(param);
  throw_event_pattern(_leave_pattern, region, ButtonHandle::none());
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::transmit_data
//       Access: Public
//  Description: Convert mouse data into a mouseWatcher matrix
////////////////////////////////////////////////////////////////////
void MouseWatcher::
transmit_data(AllTransitionsWrapper &data) {
  // Get the current mouse position.
  const Vec3DataTransition *xyz;
  if (!get_transition_into(xyz, data, _xyz_type)) {
    if (_has_mouse) {
      // Hide the mouse pointer.
      if (!_geometry.is_null()) {
        _geometry->set_transition(new PruneTransition);
      }
    }

    _has_mouse = false;
    // If the mouse is outside the window, do nothing; let all the
    // events continue down the pipe unmolested.
    clear_current_regions();
    return;
  }

  LVecBase3f p = xyz->get_value();
  _mouse.set(p[0], p[1]);

  if (!_geometry.is_null()) {
    // Transform the mouse pointer.
    LMatrix4f mat = LMatrix4f::translate_mat(p[0], 0, p[1]);
    _geometry->set_transition(new TransformTransition(mat));
    if (!_has_mouse) {
      // Show the mouse pointer.
      _geometry->clear_transition(PruneTransition::get_class_type());
    }
  }

  _has_mouse = true;

  VRegions regions;
  get_over_regions(regions, _mouse);
  set_current_regions(regions);

  _suppress_flags = 0;
  if (_preferred_region != (MouseWatcherRegion *)NULL) {
    _suppress_flags = _preferred_region->get_suppress_flags();
  }

  // Look for button events.
  const ButtonEventDataTransition *b;
  if (get_transition_into(b, data, _button_events_type)) {
    ButtonEventDataTransition::const_iterator bi;
    for (bi = b->begin(); bi != b->end(); ++bi) {
      const ButtonEvent &be = (*bi);
      _mods.add_event(be);

      if (be._down) {
        press(be._button);
      } else {
        release(be._button);
      }
    }
  }

  if ((_suppress_flags & MouseWatcherRegion::SF_mouse_position) != 0) {
    // Suppress the mouse position.
    data.clear_transition(_xyz_type);
    data.clear_transition(_pixel_xyz_type);
  }
  int suppress_buttons = (_suppress_flags & MouseWatcherRegion::SF_any_button);
  if (suppress_buttons == MouseWatcherRegion::SF_any_button) {
    // Suppress all buttons.
    data.clear_transition(_button_events_type);

  } else if (suppress_buttons != 0) {
    // Suppress some buttons.
    const ButtonEventDataTransition *b;
    ButtonEventDataTransition *new_b = (ButtonEventDataTransition *)NULL;

    if (get_transition_into(b, data, _button_events_type)) {
      ButtonEventDataTransition::const_iterator bi;
      for (bi = b->begin(); bi != b->end(); ++bi) {
        const ButtonEvent &be = (*bi);
        bool suppress = true;

        if (MouseButton::is_mouse_button(be._button)) {
          suppress = ((suppress_buttons & MouseWatcherRegion::SF_mouse_button) != 0);
        } else {
          suppress = ((suppress_buttons & MouseWatcherRegion::SF_other_button) != 0);
        }

        if (!suppress) {
          // Don't suppress this button event; pass it through.
          if (new_b == (ButtonEventDataTransition *)NULL) {
            new_b = new ButtonEventDataTransition;
          }
          new_b->push_back(be);
        }
      }
    }

    if (new_b == (ButtonEventDataTransition *)NULL) {
      data.clear_transition(_button_events_type);
    } else {
      data.set_transition(_button_events_type, new_b);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcher::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "MouseWatcher",
                DataNode::get_class_type());

  Vec3DataTransition::init_type();
  register_data_transition(_xyz_type, "XYZ",
                           Vec3DataTransition::get_class_type());
  register_data_transition(_pixel_xyz_type, "PixelXYZ",
                           Vec3DataTransition::get_class_type());
  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
                           ButtonEventDataTransition::get_class_type());
}
