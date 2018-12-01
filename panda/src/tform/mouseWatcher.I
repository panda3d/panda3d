/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseWatcher.I
 * @author drose
 * @date 2002-03-12
 */

/**
 * Returns true if the mouse is anywhere within the window, false otherwise.
 * Also see is_mouse_open().
 */
INLINE bool MouseWatcher::
has_mouse() const {
  return _has_mouse;
}

/**
 * Returns true if the mouse is within the window and not over some particular
 * MouseWatcherRegion that is marked to suppress mouse events; that is, that
 * the mouse is in open space within the window.
 */
INLINE bool MouseWatcher::
is_mouse_open() const {
  return _has_mouse && (_internal_suppress & MouseWatcherRegion::SF_mouse_position) == 0;
}

/**
 * It is only valid to call this if has_mouse() returns true.  If so, this
 * returns the current position of the mouse within the window.
 */
INLINE const LPoint2 &MouseWatcher::
get_mouse() const {
#ifndef NDEBUG
  static LPoint2 bogus_mouse(0.0f, 0.0f);
  nassertr(_has_mouse, bogus_mouse);
#endif
  return _mouse;
}

/**
 * It is only valid to call this if has_mouse() returns true.  If so, this
 * returns the current X position of the mouse within the window.
 */
INLINE PN_stdfloat MouseWatcher::
get_mouse_x() const {
  nassertr(_has_mouse, 0.0f);
  return _mouse[0];
}

/**
 * It is only valid to call this if has_mouse() returns true.  If so, this
 * returns the current Y position of the mouse within the window.
 */
INLINE PN_stdfloat MouseWatcher::
get_mouse_y() const {
  nassertr(_has_mouse, 0.0f);
  return _mouse[1];
}

/**
 * Sets the frame of the MouseWatcher.  See the next flavor of this method for
 * a more verbose explanation.
 */
INLINE void MouseWatcher::
set_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  set_frame(LVecBase4(left, right, bottom, top));
}

/**
 * Sets the frame of the MouseWatcher.  This determines the coordinate space
 * in which the MouseWatcherRegions should be expected to live.  Normally,
 * this is left at -1, 1, -1, 1, which is the default setting, and matches the
 * mouse coordinate range.
 *
 * Whatever values you specify here indicate the shape of the full screen, and
 * the MouseWatcherRegions will be given in coordinate space matching it.  For
 * instance, if you specify (0, 1, 0, 1), then a MouseWatcherRegion with the
 * frame (0, 1, 0, .5) will cover the lower half of the screen.
 */
INLINE void MouseWatcher::
set_frame(const LVecBase4 &frame) {
  _frame = frame;
}

/**
 * Returns the frame of the MouseWatcher.  See set_frame().
 */
INLINE const LVecBase4 &MouseWatcher::
get_frame() const {
  return _frame;
}

/**
 * Returns true if the mouse is over any rectangular region, false otherwise.
 */
INLINE bool MouseWatcher::
is_over_region() const {
  return get_over_region() != nullptr;
}

/**
 * Returns true if the mouse is over any rectangular region, false otherwise.
 */
INLINE bool MouseWatcher::
is_over_region(PN_stdfloat x, PN_stdfloat y) const {
  return get_over_region(x, y) != nullptr;
}

/**
 * Returns true if the mouse is over any rectangular region, false otherwise.
 */
INLINE bool MouseWatcher::
is_over_region(const LPoint2 &pos) const {
  return get_over_region(pos) != nullptr;
}

/**
 * Returns the smallest region the mouse is currently over, or NULL if it is
 * over no region.
 */
INLINE MouseWatcherRegion *MouseWatcher::
get_over_region() const {
  return _preferred_region;
}

/**
 * Returns the smallest region the indicated point is over, or NULL if it is
 * over no region.
 */
INLINE MouseWatcherRegion *MouseWatcher::
get_over_region(PN_stdfloat x, PN_stdfloat y) const {
  return get_over_region(LPoint2(x, y));
}

/**
 * Returns true if the indicated button is currently being held down, false
 * otherwise.
 */
INLINE bool MouseWatcher::
is_button_down(ButtonHandle button) const {
  return _inactivity_state != IS_inactive && _current_buttons_down.get_bit(button.get_index());
}

/**
 * Sets the pattern string that indicates how the event names are generated
 * when a button is depressed.  This is a string that may contain any of the
 * following:
 *
 * %r  - the name of the region the mouse is over %b  - the name of the button
 * pressed.
 *
 * The event name will be based on the in_pattern string specified here, with
 * all occurrences of the above strings replaced with the corresponding
 * values.
 */
INLINE void MouseWatcher::
set_button_down_pattern(const std::string &pattern) {
  _button_down_pattern = pattern;
}

/**
 * Returns the string that indicates how event names are generated when a
 * button is depressed.  See set_button_down_pattern().
 */
INLINE const std::string &MouseWatcher::
get_button_down_pattern() const {
  return _button_down_pattern;
}

/**
 * Sets the pattern string that indicates how the event names are generated
 * when a button is released.  See set_button_down_pattern().
 */
INLINE void MouseWatcher::
set_button_up_pattern(const std::string &pattern) {
  _button_up_pattern = pattern;
}

/**
 * Returns the string that indicates how event names are generated when a
 * button is released.  See set_button_down_pattern().
 */
INLINE const std::string &MouseWatcher::
get_button_up_pattern() const {
  return _button_up_pattern;
}

/**
 * Sets the pattern string that indicates how the event names are generated
 * when a button is continuously held and generates keyrepeat "down" events.
 * This is a string that may contain any of the following:
 *
 * %r  - the name of the region the mouse is over %b  - the name of the button
 * pressed.
 *
 * The event name will be based on the in_pattern string specified here, with
 * all occurrences of the above strings replaced with the corresponding
 * values.
 */
INLINE void MouseWatcher::
set_button_repeat_pattern(const std::string &pattern) {
  _button_repeat_pattern = pattern;
}

/**
 * Returns the string that indicates how event names are names are generated
 * when a button is continuously held and generates keyrepeat "down" events.
 * See set_button_repeat_pattern().
 */
INLINE const std::string &MouseWatcher::
get_button_repeat_pattern() const {
  return _button_repeat_pattern;
}

/**
 * Sets the pattern string that indicates how the event names are generated
 * when the mouse enters a region.  This is different from within_pattern, in
 * that a mouse is only "entered" in the topmost region at a given time, while
 * it might be "within" multiple nested regions.
 */
INLINE void MouseWatcher::
set_enter_pattern(const std::string &pattern) {
  _enter_pattern = pattern;
}

/**
 * Returns the string that indicates how event names are generated when the
 * mouse enters a region.  This is different from within_pattern, in that a
 * mouse is only "entered" in the topmost region at a given time, while it
 * might be "within" multiple nested regions.
 */
INLINE const std::string &MouseWatcher::
get_enter_pattern() const {
  return _enter_pattern;
}

/**
 * Sets the pattern string that indicates how the event names are generated
 * when the mouse leaves a region.  This is different from without_pattern, in
 * that a mouse is only "entered" in the topmost region at a given time, while
 * it might be "within" multiple nested regions.
 */
INLINE void MouseWatcher::
set_leave_pattern(const std::string &pattern) {
  _leave_pattern = pattern;
}

/**
 * Returns the string that indicates how event names are generated when the
 * mouse leaves a region.  This is different from without_pattern, in that a
 * mouse is only "entered" in the topmost region at a given time, while it
 * might be "within" multiple nested regions.
 */
INLINE const std::string &MouseWatcher::
get_leave_pattern() const {
  return _leave_pattern;
}

/**
 * Sets the pattern string that indicates how the event names are generated
 * when the mouse wanders over a region.  This is different from
 * enter_pattern, in that a mouse is only "entered" in the topmost region at a
 * given time, while it might be "within" multiple nested regions.
 */
INLINE void MouseWatcher::
set_within_pattern(const std::string &pattern) {
  _within_pattern = pattern;
}

/**
 * Returns the string that indicates how event names are generated when the
 * mouse wanders over a region.  This is different from enter_pattern, in that
 * a mouse is only "entered" in the topmost region at a given time, while it
 * might be "within" multiple nested regions.
 */
INLINE const std::string &MouseWatcher::
get_within_pattern() const {
  return _within_pattern;
}

/**
 * Sets the pattern string that indicates how the event names are generated
 * when the mouse wanders out of a region.  This is different from
 * leave_pattern, in that a mouse is only "entered" in the topmost region at a
 * given time, while it might be "within" multiple nested regions.
 */
INLINE void MouseWatcher::
set_without_pattern(const std::string &pattern) {
  _without_pattern = pattern;
}

/**
 * Returns the string that indicates how event names are generated when the
 * mouse wanders out of a region.  This is different from leave_pattern, in
 * that a mouse is only "entered" in the topmost region at a given time, while
 * it might be "within" multiple nested regions.
 */
INLINE const std::string &MouseWatcher::
get_without_pattern() const {
  return _without_pattern;
}

/**
 * Sets the node that will be transformed each frame by the mouse's
 * coordinates.  It will also be hidden when the mouse goes outside the
 * window.  This can be used to implement a software mouse pointer for when a
 * hardware (or system) mouse pointer is unavailable.
 */
INLINE void MouseWatcher::
set_geometry(PandaNode *node) {
  _geometry = node;
}

/**
 * Returns true if a software mouse pointer has been setup via set_geometry(),
 * or false otherwise.  See set_geometry().
 */
INLINE bool MouseWatcher::
has_geometry() const {
  return !_geometry.is_null();
}

/**
 * Returns the node that has been set as the software mouse pointer, or NULL
 * if no node has been set.  See has_geometry() and set_geometry().
 */
INLINE PandaNode *MouseWatcher::
get_geometry() const {
  return _geometry;
}

/**
 * Stops the use of the software cursor set up via set_geometry().
 */
INLINE void MouseWatcher::
clear_geometry() {
  _geometry.clear();
}

/**
 * As an optimization for the C++ Gui, an extra handler can be registered with
 * a mouseWatcher so that events can be dealt with much sooner.
 */
INLINE void MouseWatcher::
set_extra_handler(EventHandler *eh) {
  _eh = eh;
}

/**
 * As an optimization for the C++ Gui, an extra handler can be registered with
 * a mouseWatcher so that events can be dealt with much sooner.
 */
INLINE EventHandler *MouseWatcher::
get_extra_handler() const {
  return _eh;
}

/**
 * Sets the buttons that should be monitored as modifier buttons for
 * generating events to the MouseWatcherRegions.
 */
INLINE void MouseWatcher::
set_modifier_buttons(const ModifierButtons &mods) {
  _mods = mods;
}

/**
 * Returns the set of buttons that are being monitored as modifier buttons, as
 * well as their current state.
 */
INLINE ModifierButtons MouseWatcher::
get_modifier_buttons() const {
  return _mods;
}

/**
 * Constrains the MouseWatcher to watching the mouse within a particular
 * indicated region of the screen.  DataNodes parented under the MouseWatcher
 * will observe the mouse and keyboard events only when the mouse is within
 * the indicated region, and the observed range will be from -1 .. 1 across
 * the region.
 *
 * Do not delete the DisplayRegion while it is owned by the MouseWatcher.
 */
INLINE void MouseWatcher::
set_display_region(DisplayRegion *dr) {
  _display_region = dr;
  _button_down_display_region = nullptr;
}

/**
 * Removes the display region constraint from the MouseWatcher, and restores
 * it to the default behavior of watching the whole window.
 */
INLINE void MouseWatcher::
clear_display_region() {
  _display_region = nullptr;
  _button_down_display_region = nullptr;
}

/**
 * Returns the display region the MouseWatcher is constrained to by
 * set_display_region(), or NULL if it is not constrained.
 */
INLINE DisplayRegion *MouseWatcher::
get_display_region() const {
  return _display_region;
}

/**
 * Returns true if the MouseWatcher has been constrained to a particular
 * region of the screen via set_display_region(), or false otherwise.  If this
 * returns true, get_display_region() may be used to return the particular
 * region.
 */
INLINE bool MouseWatcher::
has_display_region() const {
  return (_display_region != nullptr);
}

/**
 * Sets an inactivity timeout on the mouse activity.  When this timeout (in
 * seconds) is exceeded with no keyboard or mouse activity, all currently-held
 * buttons are automatically released.  This is intended to help protect
 * against people who inadvertently (or intentionally) leave a keyboard key
 * stuck down and then wander away from the keyboard.
 *
 * Also, when this timeout expires, the event specified by
 * set_inactivity_timeout_event() will be generated.
 */
INLINE void MouseWatcher::
set_inactivity_timeout(double timeout) {
  _has_inactivity_timeout = true;
  _inactivity_timeout = timeout;
  note_activity();
}

/**
 * Returns true if an inactivity timeout has been set, false otherwise.
 */
INLINE bool MouseWatcher::
has_inactivity_timeout() const {
  return _has_inactivity_timeout;
}

/**
 * Returns the inactivity timeout that has been set.  It is an error to call
 * this if has_inactivity_timeout() returns false.
 */
INLINE double MouseWatcher::
get_inactivity_timeout() const {
  nassertr(_has_inactivity_timeout, 0.0);
  return _inactivity_timeout;
}

/**
 * Removes the inactivity timeout and restores the MouseWatcher to its default
 * behavior of allowing a key to be held indefinitely.
 */
INLINE void MouseWatcher::
clear_inactivity_timeout() {
  _has_inactivity_timeout = false;
  _inactivity_timeout = 0.0;
  note_activity();
}

/**
 * Specifies the event string that will be generated when the inactivity
 * timeout counter expires.  See set_inactivity_timeout().
 */
INLINE void MouseWatcher::
set_inactivity_timeout_event(const std::string &event) {
  _inactivity_timeout_event = event;
}

/**
 * Returns the event string that will be generated when the inactivity timeout
 * counter expires.  See set_inactivity_timeout().
 */
INLINE const std::string &MouseWatcher::
get_inactivity_timeout_event() const {
  return _inactivity_timeout_event;
}

/**
 * Called internally to indicate the mouse pointer has moved within the
 * indicated region's boundaries.
 */
INLINE void MouseWatcher::
within_region(MouseWatcherRegion *region, const MouseWatcherParameter &param) {
  region->within_region(param);
  throw_event_pattern(_within_pattern, region, ButtonHandle::none());
  if (_enter_multiple) {
    enter_region(region, param);
  }
}

/**
 * Called internally to indicate the mouse pointer has moved outside of the
 * indicated region's boundaries.
 */
INLINE void MouseWatcher::
without_region(MouseWatcherRegion *region, const MouseWatcherParameter &param) {
  if (_enter_multiple) {
    exit_region(region, param);
  }
  region->without_region(param);
  throw_event_pattern(_without_pattern, region, ButtonHandle::none());
}

/**
 * Clears the mouse trail log.  This does not prevent further accumulation of
 * the log given future events.
 */
INLINE void MouseWatcher::
clear_trail_log() {
  _trail_log->clear();
}

/**
 * Obtain the mouse trail log.  This is a PointerEventList.  Does not make a
 * copy, therefore, this PointerEventList will be updated each time
 * process_events gets called.
 *
 * To use trail logging, you need to enable the generation of pointer events
 * in the GraphicsWindowInputDevice and set the trail log duration in the
 * MouseWatcher.  Otherwise, the trail log will be empty.
 */
INLINE CPT(PointerEventList) MouseWatcher::
get_trail_log() const {
  return _trail_log;
}

/**
 * This counter indicates how many events were added to the trail log this
 * frame.  The trail log is updated once per frame, during the process_events
 * operation.
 */
INLINE size_t MouseWatcher::
num_trail_recent() const {
  return _num_trail_recent;
}
