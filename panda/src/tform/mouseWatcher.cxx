// Filename: mouseWatcher.cxx
// Created by:  drose (13Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "mouseWatcher.h"
#include "config_tform.h"

#include <mouse.h>
#include <mouseData.h>
#include <modifierButtons.h>
#include <buttonEventDataTransition.h>
#include <buttonEventDataAttribute.h>
#include <modifierButtonDataTransition.h>
#include <modifierButtonDataAttribute.h>
#include <keyboardButton.h>
#include <mouseButton.h>
#include <throw_event.h>
#include <pruneTransition.h>
#include <transformTransition.h>

TypeHandle MouseWatcher::_type_handle;

TypeHandle MouseWatcher::_mods_type;
TypeHandle MouseWatcher::_xyz_type;
TypeHandle MouseWatcher::_button_events_type;

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::Constructor
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
MouseWatcher::
MouseWatcher(const string &name) : DataNode(name) {
  _has_mouse = false;
  _current_region = (MouseWatcherRegion *)NULL;
  _button_down_region = (MouseWatcherRegion *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::Destructor
//       Access: Public, Scheme
//  Description:
////////////////////////////////////////////////////////////////////
MouseWatcher::
~MouseWatcher() {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::add_region
//       Access: Public, Scheme
//  Description: Adds the indicated region to the set of regions that
//               are to be watched.  Returns true if it was
//               successfully added, or false if it was already on the
//               list.
////////////////////////////////////////////////////////////////////
bool MouseWatcher::
add_region(MouseWatcherRegion *region) {
  return _regions.insert(region).second;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::has_region
//       Access: Public, Scheme
//  Description: Returns true if the indicated region has already been
//               added to the MouseWatcher, false otherwise.
////////////////////////////////////////////////////////////////////
bool MouseWatcher::
has_region(MouseWatcherRegion *region) const {
  return _regions.count(region) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::remove_region 
//       Access: Public, Scheme
//  Description: Removes the indicated region from the Watcher.
//               Returns true if it was successfully removed, or false
//               if it wasn't there in the first place.
////////////////////////////////////////////////////////////////////
bool MouseWatcher::
remove_region(MouseWatcherRegion *region) {
  return _regions.erase(region) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_over_region 
//       Access: Public, Scheme
//  Description: Returns the smallest region the indicated point is
//               over, or NULL if it is over no region.
////////////////////////////////////////////////////////////////////
MouseWatcherRegion *MouseWatcher::
get_over_region(const LPoint2f &pos) const {
  MouseWatcherRegion *over_region = (MouseWatcherRegion *)NULL;
  float over_area = 0.0;

  Regions::const_iterator ri;
  for (ri = _regions.begin(); ri != _regions.end(); ++ri) {
    MouseWatcherRegion *region = (*ri);
    const LVecBase4f &frame = region->get_frame();

    if (region->get_active() &&
	pos[0] >= frame[0] && pos[0] <= frame[1] &&
	pos[1] >= frame[2] && pos[1] <= frame[3]) {

      // We're over this region.  Is it the smallest?
      if (over_region == (MouseWatcherRegion *)NULL ||
	  region->get_area() < over_area) {
	over_region = region;
	over_area = region->get_area();
      }
    }
  }

  return over_region;
}


////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MouseWatcher::
output(ostream &out) const {
  DataNode::output(out);
  out << " (" << _regions.size() << " regions)";
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::write
//       Access: Public
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
  if (region != _current_region) {
    if (_current_region != (MouseWatcherRegion *)NULL) {
      throw_event_pattern(_leave_pattern, _current_region);
    }
    _current_region = region;
    if (_current_region != (MouseWatcherRegion *)NULL) {
      throw_event_pattern(_enter_pattern, _current_region);
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
		    const string &button_name) {
  if (pattern.empty()) {
    return;
  }

  string event;
  for (size_t p = 0; p < pattern.size(); ++p) {
    if (pattern[p] == '%') {
      string cmd = pattern.substr(p + 1, 1);
      p++;
      if (cmd == "r") {
	event += region->get_name();

      } else if (cmd == "b") {
	event += button_name;

      } else {
	tform_cat.error()
	  << "Invalid symbol in event_pattern: %" << cmd << "\n";
      }
    } else {
      event += pattern[p];
    }
  }

  if (!event.empty()) {
    throw_event(event);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::transmit_data
//       Access: Public
//  Description: Convert mouse data into a mouseWatcher matrix 
////////////////////////////////////////////////////////////////////
void MouseWatcher::
transmit_data(NodeAttributes &data) {
  // Get the current mouse position.
  const Vec3DataAttribute *xyz;
  if (!get_attribute_into(xyz, data, _xyz_type)) {
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

  set_current_region(get_over_region(_mouse));

  // Look for button events.
  const ButtonEventDataAttribute *b;
  if (get_attribute_into(b, data, _button_events_type)) {
    ButtonEventDataAttribute::const_iterator bi;
    for (bi = b->begin(); bi != b->end(); ++bi) {
      const ButtonEvent &be = (*bi);

      if (!be._down) {
	// Button up.  Send the up event associated with the region we
	// were over when the button went down.

	// There is some danger of losing button-up events here.  If
	// more than one button goes down together, we won't detect
	// both the of the button-up events properly.

	if (_button_down_region != (MouseWatcherRegion *)NULL) {
	  throw_event_pattern(_button_up_pattern, _button_down_region,
			      be._button.get_name());
	}
	_button_down_region = (MouseWatcherRegion *)NULL;
	
      } else {
	// Button down.
	_button_down_region = _current_region;
	if (_current_region != (MouseWatcherRegion *)NULL) {
	  throw_event_pattern(_button_down_pattern, _button_down_region,
			      be._button.get_name());
	}
      }
    }
  }

  if (_button_down_region != (MouseWatcherRegion *)NULL) {
    // We're currently holding down a button.  This is the effective
    // region that determines whether we suppress below.
    if (_button_down_region->get_suppress_below()) {
      data.clear();
    }

  } else if (_current_region != (MouseWatcherRegion *)NULL) {
    // We're not holding down a button, but we are within a region.
    // Use this region to determine whether we suppress below.
    if (_current_region->get_suppress_below()) {
      data.clear();
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

  ModifierButtonDataTransition::init_type();
  register_data_transition(_mods_type, "ModifierButtons",
			   ModifierButtonDataTransition::get_class_type());
  Vec3DataTransition::init_type();
  register_data_transition(_xyz_type, "XYZ",
			   Vec3DataTransition::get_class_type());
  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
			   ButtonEventDataTransition::get_class_type());
}
