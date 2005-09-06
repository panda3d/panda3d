// Filename: mouseWatcher.I
// Created by:  drose (12Mar02)
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
//     Function: MouseWatcher::has_mouse
//       Access: Published
//  Description: Returns true if the mouse is anywhere within the
//               window, false otherwise.  Also see is_mouse_open().
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcher::
has_mouse() const {
  return _has_mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::is_mouse_open
//       Access: Published
//  Description: Returns true if the mouse is within the window and
//               not over some particular MouseWatcherRegion that is
//               marked to suppress mouse events; that is, that the
//               mouse is in open space within the window.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcher::
is_mouse_open() const {
  return _has_mouse && (_internal_suppress & MouseWatcherRegion::SF_mouse_position) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_mouse
//       Access: Published
//  Description: It is only valid to call this if has_mouse() returns
//               true.  If so, this returns the current position of
//               the mouse within the window.
////////////////////////////////////////////////////////////////////
INLINE const LPoint2f &MouseWatcher::
get_mouse() const {
#ifndef NDEBUG
  static LPoint2f bogus_mouse(0.0f, 0.0f);
  nassertr(_has_mouse, bogus_mouse);
#endif
  return _mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_mouse_x
//       Access: Published
//  Description: It is only valid to call this if has_mouse() returns
//               true.  If so, this returns the current X position of
//               the mouse within the window.
////////////////////////////////////////////////////////////////////
INLINE float MouseWatcher::
get_mouse_x() const {
  nassertr(_has_mouse, 0.0f);
  return _mouse[0];
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_mouse_y
//       Access: Published
//  Description: It is only valid to call this if has_mouse() returns
//               true.  If so, this returns the current Y position of
//               the mouse within the window.
////////////////////////////////////////////////////////////////////
INLINE float MouseWatcher::
get_mouse_y() const {
  nassertr(_has_mouse, 0.0f);
  return _mouse[1];
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::is_over_region
//       Access: Published
//  Description: Returns true if the mouse is over any rectangular
//               region, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcher::
is_over_region() const {
  return get_over_region() != (MouseWatcherRegion *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::is_over_region
//       Access: Published
//  Description: Returns true if the mouse is over any rectangular
//               region, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcher::
is_over_region(float x, float y) const {
  return get_over_region(x, y) != (MouseWatcherRegion *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::is_over_region
//       Access: Published
//  Description: Returns true if the mouse is over any rectangular
//               region, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcher::
is_over_region(const LPoint2f &pos) const {
  return get_over_region(pos) != (MouseWatcherRegion *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_over_region
//       Access: Published
//  Description: Returns the smallest region the mouse is currently
//               over, or NULL if it is over no region.
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherRegion *MouseWatcher::
get_over_region() const {
  return _preferred_region;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_over_region
//       Access: Published
//  Description: Returns the smallest region the indicated point is
//               over, or NULL if it is over no region.
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherRegion *MouseWatcher::
get_over_region(float x, float y) const {
  return get_over_region(LPoint2f(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_button_down_pattern
//       Access: Published
//  Description: Sets the pattern string that indicates how the event
//               names are generated when a button is depressed.  This
//               is a string that may contain any of the following:
//
//                  %r  - the name of the region the mouse is over
//                  %b  - the name of the button pressed.
//
//               The event name will be based on the in_pattern
//               string specified here, with all occurrences of the
//               above strings replaced with the corresponding values.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_button_down_pattern(const string &pattern) {
  _button_down_pattern = pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_button_down_pattern
//       Access: Published
//  Description: Returns the string that indicates how event names are
//               generated when a button is depressed.  See
//               set_button_down_pattern().
////////////////////////////////////////////////////////////////////
INLINE const string &MouseWatcher::
get_button_down_pattern() const {
  return _button_down_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_button_up_pattern
//       Access: Published
//  Description: Sets the pattern string that indicates how the event
//               names are generated when a button is released.  See
//               set_button_down_pattern().
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_button_up_pattern(const string &pattern) {
  _button_up_pattern = pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_button_up_pattern
//       Access: Published
//  Description: Returns the string that indicates how event names are
//               generated when a button is released.  See
//               set_button_down_pattern().
////////////////////////////////////////////////////////////////////
INLINE const string &MouseWatcher::
get_button_up_pattern() const {
  return _button_up_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_enter_pattern
//       Access: Published
//  Description: Sets the pattern string that indicates how the event
//               names are generated when the mouse enters a region.
//               This is different from within_pattern, in that a
//               mouse is only "entered" in the topmost region at a
//               given time, while it might be "within" multiple
//               nested regions.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_enter_pattern(const string &pattern) {
  _enter_pattern = pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_enter_pattern
//       Access: Published
//  Description: Returns the string that indicates how event names are
//               generated when the mouse enters a region.  This is
//               different from within_pattern, in that a mouse is
//               only "entered" in the topmost region at a given time,
//               while it might be "within" multiple nested regions.
////////////////////////////////////////////////////////////////////
INLINE const string &MouseWatcher::
get_enter_pattern() const {
  return _enter_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_leave_pattern
//       Access: Published
//  Description: Sets the pattern string that indicates how the event
//               names are generated when the mouse leaves a region.
//               This is different from without_pattern, in that a
//               mouse is only "entered" in the topmost region at a
//               given time, while it might be "within" multiple
//               nested regions.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_leave_pattern(const string &pattern) {
  _leave_pattern = pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_leave_pattern
//       Access: Published
//  Description: Returns the string that indicates how event names are
//               generated when the mouse leaves a region.  This is
//               different from without_pattern, in that a mouse is
//               only "entered" in the topmost region at a given time,
//               while it might be "within" multiple nested regions.
////////////////////////////////////////////////////////////////////
INLINE const string &MouseWatcher::
get_leave_pattern() const {
  return _leave_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_within_pattern
//       Access: Published
//  Description: Sets the pattern string that indicates how the event
//               names are generated when the mouse wanders over a
//               region.  This is different from enter_pattern, in
//               that a mouse is only "entered" in the topmost region
//               at a given time, while it might be "within" multiple
//               nested regions.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_within_pattern(const string &pattern) {
  _within_pattern = pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_within_pattern
//       Access: Published
//  Description: Returns the string that indicates how event names are
//               generated when the mouse wanders over a region.  This
//               is different from enter_pattern, in that a mouse is
//               only "entered" in the topmost region at a given time,
//               while it might be "within" multiple nested regions.
////////////////////////////////////////////////////////////////////
INLINE const string &MouseWatcher::
get_within_pattern() const {
  return _within_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_without_pattern
//       Access: Published
//  Description: Sets the pattern string that indicates how the event
//               names are generated when the mouse wanders out of a
//               region.  This is different from leave_pattern, in
//               that a mouse is only "entered" in the topmost region
//               at a given time, while it might be "within" multiple
//               nested regions.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_without_pattern(const string &pattern) {
  _without_pattern = pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_without_pattern
//       Access: Published
//  Description: Returns the string that indicates how event names are
//               generated when the mouse wanders out of a region.
//               This is different from leave_pattern, in that a mouse
//               is only "entered" in the topmost region at a given
//               time, while it might be "within" multiple nested
//               regions.
////////////////////////////////////////////////////////////////////
INLINE const string &MouseWatcher::
get_without_pattern() const {
  return _without_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_geometry
//       Access: Published
//  Description: Sets the node that will be transformed each frame by
//               the mouse's coordinates.  It will also be hidden when
//               the mouse goes outside the window.  This can be used
//               to implement a software mouse pointer for when a
//               hardware (or system) mouse pointer is unavailable.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_geometry(PandaNode *node) {
  _geometry = node;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::has_geometry
//       Access: Published
//  Description: Returns true if a software mouse pointer has been
//               setup via set_geometry(), or false otherwise.  See
//               set_geometry().
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcher::
has_geometry() const {
  return !_geometry.is_null();
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_geometry
//       Access: Published
//  Description: Returns the node that has been set as the software
//               mouse pointer, or NULL if no node has been set.  See
//               has_geometry() and set_geometry().
////////////////////////////////////////////////////////////////////
INLINE PandaNode *MouseWatcher::
get_geometry() const {
  return _geometry;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::clear_geometry
//       Access: Published
//  Description: Stops the use of the software cursor set up via
//               set_geometry().
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
clear_geometry() {
  _geometry.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_extra_handler
//       Access: Published
//  Description: As an optimization for the C++ Gui, an extra handler
//               can be registered with a mouseWatcher so that events
//               can be dealt with much sooner.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_extra_handler(EventHandler *eh) {
  _eh = eh;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_extra_handler
//       Access: Published
//  Description: As an optimization for the C++ Gui, an extra handler
//               can be registered with a mouseWatcher so that events
//               can be dealt with much sooner.
////////////////////////////////////////////////////////////////////
INLINE EventHandler *MouseWatcher::
get_extra_handler() const {
  return _eh;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_modifier_buttons
//       Access: Public
//  Description: Sets the buttons that should be monitored as modifier
//               buttons for generating events to the
//               MouseWatcherRegions.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_modifier_buttons(const ModifierButtons &mods) {
  _mods = mods;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_modifier_buttons
//       Access: Published
//  Description: Returns the set of buttons that are being monitored
//               as modifier buttons, as well as their current state.
////////////////////////////////////////////////////////////////////
INLINE ModifierButtons MouseWatcher::
get_modifier_buttons() const {
  return _mods;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::set_display_region
//       Access: Published
//  Description: Constrains the MouseWatcher to watching the mouse
//               within a particular indicated region of the screen.
//               DataNodes parented under the MouseWatcher will
//               observe the mouse and keyboard events only when the
//               mouse is within the indicated region, and the
//               observed range will be from -1 .. 1 across the
//               region.
//
//               Do not delete the DisplayRegion while it is owned by
//               the MouseWatcher.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
set_display_region(DisplayRegion *dr) {
  _display_region = dr;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::clear_display_region
//       Access: Published
//  Description: Removes the display region constraint from the
//               MouseWatcher, and restores it to the default behavior
//               of watching the whole window.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
clear_display_region() {
  _display_region = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::get_display_region
//       Access: Published
//  Description: Returns the display region the MouseWatcher is
//               constrained to by set_display_region(), or NULL if it
//               is not constrained.
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *MouseWatcher::
get_display_region() const {
  return _display_region;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::has_display_region
//       Access: Published
//  Description: Returns true if the MouseWatcher has been constrained
//               to a particular region of the screen via
//               set_display_region(), or false otherwise.  If this
//               returns true, get_display_region() may be used to
//               return the particular region.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcher::
has_display_region() const {
  return (_display_region != (DisplayRegion *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::within_region
//       Access: Protected
//  Description: Called internally to indicate the mouse pointer has
//               moved within the indicated region's boundaries.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
within_region(MouseWatcherRegion *region, const MouseWatcherParameter &param) {
  region->within(param);
  throw_event_pattern(_within_pattern, region, ButtonHandle::none());
  if (_enter_multiple) {
    enter_region(region, param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcher::without_region
//       Access: Protected
//  Description: Called internally to indicate the mouse pointer has
//               moved outside of the indicated region's boundaries.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcher::
without_region(MouseWatcherRegion *region, const MouseWatcherParameter &param) {
  if (_enter_multiple) {
    exit_region(region, param);
  }
  region->without(param);
  throw_event_pattern(_without_pattern, region, ButtonHandle::none());
}
