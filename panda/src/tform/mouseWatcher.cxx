// Filename: mouseWatcher.cxx
// Created by:  drose (13Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "mouseWatcher.h"
#include "config_tform.h"

#include <mouse.h>
#include <mouseData.h>
#include <buttonEventDataTransition.h>
#include <buttonEventDataAttribute.h>
#include <keyboardButton.h>
#include <mouseButton.h>
#include <throw_event.h>
#include <eventParameter.h>
#include <pruneTransition.h>
#include <transformTransition.h>

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
//     Function: MouseWatcher::add_region
//       Access: Published
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
//       Access: Published
//  Description: Returns true if the indicated region has already been
//               added to the MouseWatcher, false otherwise.
////////////////////////////////////////////////////////////////////
bool MouseWatcher::
has_region(MouseWatcherRegion *region) const {
  return _regions.count(region) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::remove_region 
//       Access: Published
//  Description: Removes the indicated region from the Watcher.
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
  return _regions.erase(region) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::find_region 
//       Access: Published
//  Description: Returns a pointer to the first region found with the
//               indicated name.  If multiple regions share the same
//               name, the one that is returned is indeterminate.
////////////////////////////////////////////////////////////////////
MouseWatcherRegion *MouseWatcher::
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
  if (region != (MouseWatcherRegion *)NULL) {
    region->test_ref_count_integrity();
  }

  string event;
  for (size_t p = 0; p < pattern.size(); ++p) {
    if (pattern[p] == '%') {
      string cmd = pattern.substr(p + 1, 1);
      p++;
      if (cmd == "r") {
	if (region != (MouseWatcherRegion *)NULL) {
	  event += region->get_name();
	}

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
    throw_event(event, EventParameter(region), EventParameter(button_name));
    if (_eh != (EventHandler*)0L)
      throw_event_directly(*_eh, event, EventParameter(region),
			   EventParameter(button_name));
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
	// both of the button-up events properly.

	// We should probably throw a different button_up event based
	// on whether the _current_region is NULL or not, so the
	// calling code can differentiate between button_up within the
	// starting region, and button_up outside the region.
	// Presently, changing this will break the GUI code.
	if (_button_down_region != (MouseWatcherRegion *)NULL) {
	  throw_event_pattern(_button_up_pattern, _button_down_region,
			      be._button.get_name());
	}
	_button_down = false;
	
      } else {
	// Button down.

	if (!_button_down) {
	  _button_down_region = _current_region;
	}
	_button_down = true;
	if (_button_down_region != (MouseWatcherRegion *)NULL) {
	  throw_event_pattern(_button_down_pattern, _button_down_region,
			      be._button.get_name());
	}
      }
    }
  }

  bool suppress_below = false;
  if (_current_region != (MouseWatcherRegion *)NULL) {
    suppress_below = _current_region->get_suppress_below();
  }

  if (suppress_below) {
    // We used to suppress *everything* below, but on reflection we
    // really only want to suppress the mouse position information.
    // Button events must still get through.

    data.clear_attribute(_xyz_type);
    data.clear_attribute(_pixel_xyz_type);
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
