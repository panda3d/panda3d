/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonThrower.I
 * @author drose
 * @date 2003-12-26
 */

/**
 * Specifies the generic event that is generated (if any) each time a key or
 * button is depressed.  Unlike the specific events that are unique to each
 * key, this same event name is used for *all* button events, and the name of
 * the button pressed (possibly with modifier prefixes) will be sent as a
 * parameter.
 *
 * If this string is empty, no event is generated.  It is possible to generate
 * both generic events and specific events for the same button.
 *
 * See also set_keystroke_event().
 */
INLINE void ButtonThrower::
set_button_down_event(const std::string &button_down_event) {
  _button_down_event = button_down_event;
}

/**
 * Returns the button_down_event that has been set on this ButtonThrower.  See
 * set_button_down_event().
 */
INLINE const std::string &ButtonThrower::
get_button_down_event() const {
  return _button_down_event;
}

/**
 * Specifies the generic event that is generated (if any) each time a key or
 * button is released.  See set_button_down_event().
 */
INLINE void ButtonThrower::
set_button_up_event(const std::string &button_up_event) {
  _button_up_event = button_up_event;
}

/**
 * Returns the button_up_event that has been set on this ButtonThrower.  See
 * set_button_up_event().
 */
INLINE const std::string &ButtonThrower::
get_button_up_event() const {
  return _button_up_event;
}

/**
 * Specifies the generic event that is generated (if any) repeatedly while a
 * key or button is held down.  Unlike the specific events that are unique to
 * each key, this same event name is used for *all* button events, and the
 * name of the button pressed (possibly with modifier prefixes) will be sent
 * as a parameter.
 *
 * If this string is empty, no event is generated.  It is possible to generate
 * both generic events and specific events for the same button.
 *
 * See also set_keystroke_event().
 */
INLINE void ButtonThrower::
set_button_repeat_event(const std::string &button_repeat_event) {
  _button_repeat_event = button_repeat_event;
}

/**
 * Returns the button_repeat_event that has been set on this ButtonThrower.
 * See set_button_repeat_event().
 */
INLINE const std::string &ButtonThrower::
get_button_repeat_event() const {
  return _button_repeat_event;
}

/**
 * Specifies the event that is generated (if any) for each keystroke that is
 * received.  A keystroke is different than a button event: it represents the
 * semantic meaning of the sequence of keys that have been pressed.  For
 * instance, pressing shift and 4 together will generate the button event
 * "shift-4", but it will generate the keystroke "$".
 *
 * If a key is held down, keyrepeat will cause the same keystroke event to be
 * generated repeatedly.  This is different from the corresponding down event,
 * which will only be generated once, followed by a number of button repeat
 * events.
 *
 * This event is generated with a single wstring parameter, which is a one-
 * character string that contains the keystroke generated.  If this event
 * string is empty, no event is generated.
 *
 * See also set_button_down_event().
 */
INLINE void ButtonThrower::
set_keystroke_event(const std::string &keystroke_event) {
  _keystroke_event = keystroke_event;
}

/**
 * Returns the keystroke_event that has been set on this ButtonThrower.  See
 * set_keystroke_event().
 */
INLINE const std::string &ButtonThrower::
get_keystroke_event() const {
  return _keystroke_event;
}

/**
 * Specifies the event that is generated (if any) for each IME candidate
 * string event received.  Events of this nature are received only when the
 * user is entering data using a Microsoft Input Method Editor, typically used
 * for Asian languages such as Japanese or Korean.
 *
 * If you are designing a typing user interface, you should track this event
 * to support the use of the IME.  In response to this event, you should
 * display the candidate string in the entry box, with the appropriate
 * sections highlighted, so the user can scroll through the available choices.
 *
 * This event is generated with four parameters, in order: the candidate
 * string, the character at which to start the highlight, the character at
 * which to end the highlight, and the current cursor position.
 */
INLINE void ButtonThrower::
set_candidate_event(const std::string &candidate_event) {
  _candidate_event = candidate_event;
}

/**
 * Returns the candidate_event that has been set on this ButtonThrower.  See
 * set_candidate_event().
 */
INLINE const std::string &ButtonThrower::
get_candidate_event() const {
  return _candidate_event;
}

/**
 * Specifies the event that is generated (if any) each time the mouse is moved
 * within the window.
 */
INLINE void ButtonThrower::
set_move_event(const std::string &move_event) {
  _move_event = move_event;
}

/**
 * Returns the move_event that has been set on this ButtonThrower.  See
 * set_move_event().
 */
INLINE const std::string &ButtonThrower::
get_move_event() const {
  return _move_event;
}

/**
 * Like set_button_down_event, but uses the raw, untransformed scan key from
 * the operating system.  This uses buttons that are independent of the user's
 * selected keyboard layout.
 */
INLINE void ButtonThrower::
set_raw_button_down_event(const std::string &raw_button_down_event) {
  _raw_button_down_event = raw_button_down_event;
}

/**
 * Returns the raw_button_down_event that has been set on this ButtonThrower.
 * See set_raw_button_down_event().
 */
INLINE const std::string &ButtonThrower::
get_raw_button_down_event() const {
  return _raw_button_down_event;
}

/**
 * Specifies the generic event that is generated (if any) each time a key or
 * button is released.  See set_raw_button_down_event().
 */
INLINE void ButtonThrower::
set_raw_button_up_event(const std::string &raw_button_up_event) {
  _raw_button_up_event = raw_button_up_event;
}

/**
 * Returns the raw_button_up_event that has been set on this ButtonThrower.
 * See set_raw_button_up_event().
 */
INLINE const std::string &ButtonThrower::
get_raw_button_up_event() const {
  return _raw_button_up_event;
}

/**
 * Sets the prefix which is prepended to all specific event names (that is,
 * event names generated from the button name itself, as opposed to the
 * generic event names like set_button_down_event) thrown by this object.
 */
INLINE void ButtonThrower::
set_prefix(const std::string &prefix) {
  _prefix = prefix;
}

/**
 * Returns the prefix that has been set on this ButtonThrower.  See
 * set_prefix().
 */
INLINE const std::string &ButtonThrower::
get_prefix() const {
  return _prefix;
}

/**
 * Sets the flag that indicates whether specific events (events prefixed by
 * set_prefix, and based on the event name) should be generated at all.  This
 * is true by default, but may be disabled if you are only interested in the
 * generic events (for instance, events like set_button_down_event).
 */
INLINE void ButtonThrower::
set_specific_flag(bool specific_flag) {
  _specific_flag = specific_flag;
}

/**
 * Returns the flag that indicates whether specific events should be
 * generated.  See set_specific_flag().
 */
INLINE bool ButtonThrower::
get_specific_flag() const {
  return _specific_flag;
}

/**
 * Sets the flag that indicates whether the time of the button event should be
 * passed as a parameter or not.  When this is true, an additional parameter
 * is generated on each event (before all the parameters named by
 * add_parameter) that consists of a single double value, and reflects the
 * time the button was pressed or released, as a value from
 * ClockObject::get_global_clock().
 */
INLINE void ButtonThrower::
set_time_flag(bool time_flag) {
  _time_flag = time_flag;
}

/**
 * Returns the flag that indicates whether the time of the button event should
 * be passed as a parameter.
 */
INLINE bool ButtonThrower::
get_time_flag() const {
  return _time_flag;
}

/**
 * Returns the set of ModifierButtons that the ButtonThrower will consider
 * important enough to prepend the event name with.  Normally, this set will
 * be empty, and the ButtonThrower will therefore ignore all ModifierButtons
 * attached to the key events, but if one or more buttons have been added to
 * this set, and those modifier buttons are set on the button event, then the
 * event name will be prepended with the names of the modifier buttons.
 */
INLINE const ModifierButtons &ButtonThrower::
get_modifier_buttons() const {
  return _mods;
}

/**
 * Changes the set of ModifierButtons that the ButtonThrower will consider
 * important enough to prepend the event name with.  Normally, this set will
 * be empty, and the ButtonThrower will therefore ignore all ModifierButtons
 * attached to the key events, but if one or more buttons have been added to
 * this set, then the event name will be prepended with the names of the
 * modifier buttons.
 *
 * It is recommended that you change this setting by first calling
 * get_modifier_buttons(), making adjustments, and passing the new value to
 * set_modifier_buttons().  This way the current state of the modifier buttons
 * will not be lost.
 */
INLINE void ButtonThrower::
set_modifier_buttons(const ModifierButtons &mods) {
  _mods = mods;
}

/**
 * Sets the flag that indicates whether the ButtonThrower will only process
 * events for the explicitly named buttons or not.  Normally this is false,
 * meaning all buttons are processed; set it true to indicate that only some
 * buttons should be processed.  See add_throw_button().
 */
INLINE void ButtonThrower::
set_throw_buttons_active(bool flag) {
  _throw_buttons_active = flag;
}

/**
 * Returns the flag that indicates whether the ButtonThrower will only process
 * events for the explicitly named buttons or not.  See
 * set_throw_buttons_active().
 */
INLINE bool ButtonThrower::
get_throw_buttons_active() const {
  return _throw_buttons_active;
}
