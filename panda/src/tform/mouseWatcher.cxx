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
  _current_region = (MouseWatcherRegion *)NULL;
  _button_down_region = (MouseWatcherRegion *)NULL;
  _button_down = false;
  _eh = (EventHandler*)0L;
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
  if (region == _current_region) {
    _current_region = (MouseWatcherRegion *)NULL;
  }
  if (region == _button_down_region) {
    _button_down_region = (MouseWatcherRegion *)NULL;
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
  MouseWatcherRegion *over_region = (MouseWatcherRegion *)NULL;

  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);
    const LVecBase4f &frame = region->get_frame();

    if (region->get_active() &&
        pos[0] >= frame[0] && pos[0] <= frame[1] &&
        pos[1] >= frame[2] && pos[1] <= frame[3]) {

      // We're over this region.  Is it preferred to the other one?
      if (over_region == (MouseWatcherRegion *)NULL ||
          *region < *over_region) {
        over_region = region;
      }
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
        
        // We're over this region.  Is it preferred to the other one?
        if (over_region == (MouseWatcherRegion *)NULL ||
            *region < *over_region) {
          over_region = region;
        }
      }
    }
  }

  return over_region;
}


////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcher::
output(ostream &out) const {
  DataNode::output(out);
  out << " (" << _regions.size() << " regions)";
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
add_group(MouseWatcherGroup *group) {
  return _groups.insert(group).second;
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
remove_group(MouseWatcherGroup *group) {
  if (group->has_region(_current_region)) {
    _current_region = (MouseWatcherRegion *)NULL;
  }
  if (group->has_region(_button_down_region)) {
    _button_down_region = (MouseWatcherRegion *)NULL;
  }
  return _groups.erase(group) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_current_region
//       Access: Private
//  Description: Changes the "current" region--the one we consider the
//               mouse to be over--to the indicated one, and throws
//               whatever events are appropriate because of that.
////////////////////////////////////////////////////////////////////
void MouseWatcher::
set_current_region(MouseWatcherRegion *region) {
#ifndef NDEBUG
  if (region != (MouseWatcherRegion *)NULL) {
    region->test_ref_count_integrity();
  }
#endif
  if (region != _current_region) {
    MouseWatcherParameter param;
    param.set_modifier_buttons(_mods);
    param.set_mouse(_mouse);

    if (_current_region != (MouseWatcherRegion *)NULL) {
      _current_region->exit(param);
      throw_event_pattern(_leave_pattern, _current_region, ButtonHandle::none());
    }
    _current_region = region;
    if (_current_region != (MouseWatcherRegion *)NULL) {
      _current_region->enter(param);
      throw_event_pattern(_enter_pattern, _current_region, ButtonHandle::none());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::throw_event_for
//       Access: Private
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
//       Access: Private
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
      _button_down_region = _current_region;
    }
    _button_down = true;
    if (_button_down_region != (MouseWatcherRegion *)NULL) {
      _button_down_region->press(param);
      throw_event_pattern(_button_down_pattern, _button_down_region,
                          button);
    }
    
  } else {
    // It's a keyboard button; therefore, send the event to every
    // region that wants keyboard buttons, regardless of the mouse
    // position.
    if (_current_region != (MouseWatcherRegion *)NULL) {
      // Our current region, the one under the mouse, always get
      // all the keyboard events, even if it doesn't set its
      // keyboard flag.
      _current_region->press(param);
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
//       Access: Private
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
    // Button up.  Send the up event associated with the region we
    // were over when the button went down.
    
    // There is some danger of losing button-up events here.  If
    // more than one button goes down together, we won't detect
    // both of the button-up events properly.
    if (_button_down_region != (MouseWatcherRegion *)NULL) {
      param.set_outside(_current_region != _button_down_region);
      _button_down_region->release(param);
      throw_event_pattern(_button_up_pattern, _button_down_region,
                          button);
    }
    _button_down = false;
    
  } else {
    // It's a keyboard button; therefore, send the event to every
    // region that wants keyboard buttons, regardless of the mouse
    // position.
    if (_current_region != (MouseWatcherRegion *)NULL) {
      _current_region->release(param);
    }
    
    param.set_outside(true);
    global_keyboard_release(param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::global_keyboard_press
//       Access: Private
//  Description: Calls press() on all regions that are interested in
//               receiving global keyboard events, except for the
//               current region (which already received this one).
////////////////////////////////////////////////////////////////////
void MouseWatcher::
global_keyboard_press(const MouseWatcherParameter &param) {
  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);

    if (region != _current_region && region->get_keyboard()) {
      region->press(param);
    }
  }

  // Also check all of our sub-groups.
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    MouseWatcherGroup *group = (*gi);
    for (ri = group->_regions.begin(); ri != group->_regions.end(); ++ri) {
      MouseWatcherRegion *region = (*ri);

      if (region != _current_region && region->get_keyboard()) {
        region->press(param);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::global_keyboard_release
//       Access: Private
//  Description: Calls release() on all regions that are interested in
//               receiving global keyboard events, except for the
//               current region (which already received this one).
////////////////////////////////////////////////////////////////////
void MouseWatcher::
global_keyboard_release(const MouseWatcherParameter &param) {
  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);

    if (region != _current_region && region->get_keyboard()) {
      region->release(param);
    }
  }

  // Also check all of our sub-groups.
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    MouseWatcherGroup *group = (*gi);
    for (ri = group->_regions.begin(); ri != group->_regions.end(); ++ri) {
      MouseWatcherRegion *region = (*ri);

      if (region != _current_region && region->get_keyboard()) {
        region->release(param);
      }
    }
  }
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
    set_current_region(NULL);
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

  if (!_button_down) {
    // If the button is not currently being held down, we are free to
    // set the mouse into whichever region we like.
    set_current_region(get_over_region(_mouse));

  } else {
    // If the button *is* currently being held down, we can only move
    // the mouse into a region if the region is the same region we
    // started from.
    MouseWatcherRegion *region = get_over_region(_mouse);
    if (region == _button_down_region) {
      set_current_region(region);
    } else {
      set_current_region((MouseWatcherRegion *)NULL);
    }
  }

  _suppress_flags = 0;
  if (_current_region != (MouseWatcherRegion *)NULL) {
    _suppress_flags = _current_region->get_suppress_flags();
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
