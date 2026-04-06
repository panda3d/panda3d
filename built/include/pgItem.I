/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgItem.I
 * @author drose
 * @date 2002-03-13
 */

/**
 *
 */
INLINE void PGItem::
set_name(const std::string &name) {
  Namable::set_name(name);
  _lock.set_name(name);
}

/**
 * Returns the MouseWatcherRegion associated with this item.  Every PGItem has
 * a MouseWatcherRegion associated with it, that is created when the PGItem is
 * created; it does not change during the lifetime of the PGItem.  Even items
 * that do not have a frame have an associated MouseWatcherRegion, although it
 * will not be used in this case.
 */
INLINE PGMouseWatcherRegion *PGItem::
get_region() const {
  LightReMutexHolder holder(_lock);
  return _region;
}

/**
 * Sets the object which will be notified when the PGItem changes.  Set this
 * to NULL to disable this effect.  The PGItem does not retain ownership of
 * the pointer; it is your responsibility to ensure that the notify object
 * does not destruct.
 */
INLINE void PGItem::
set_notify(PGItemNotify *notify) {
  LightReMutexHolder holder(_lock);
  if (_notify != nullptr) {
    _notify->remove_item(this);
  }
  _notify = notify;
  if (_notify != nullptr) {
    _notify->add_item(this);
  }
}

/**
 * Returns true if there is an object configured to be notified when the
 * PGItem changes, false otherwise.
 */
INLINE bool PGItem::
has_notify() const {
  LightReMutexHolder holder(_lock);
  return (_notify != nullptr);
}

/**
 * Returns the object which will be notified when the PGItem changes, if any.
 * Returns NULL if there is no such object configured.
 */
INLINE PGItemNotify *PGItem::
get_notify() const {
  LightReMutexHolder holder(_lock);
  return _notify;
}

/**
 * Sets the bounding rectangle of the item, in local coordinates.  This is the
 * region on screen within which the mouse will be considered to be within the
 * item.  Normally, it should correspond to the bounding rectangle of the
 * visible geometry of the item.
 */
INLINE void PGItem::
set_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  set_frame(LVecBase4(left, right, bottom, top));
}

/**
 * Sets the bounding rectangle of the item, in local coordinates.  This is the
 * region on screen within which the mouse will be considered to be within the
 * item.  Normally, it should correspond to the bounding rectangle of the
 * visible geometry of the item.
 */
INLINE void PGItem::
set_frame(const LVecBase4 &frame) {
  LightReMutexHolder holder(_lock);
  if (!_has_frame || _frame != frame) {
    _has_frame = true;
    _frame = frame;
    frame_changed();
  }
}

/**
 * Returns the bounding rectangle of the item.  See set_frame().  It is an
 * error to call this if has_frame() returns false.
 */
INLINE const LVecBase4 &PGItem::
get_frame() const {
  LightReMutexHolder holder(_lock);
  nassertr(has_frame(), _frame);
  return _frame;
}

/**
 * Returns true if the item has a bounding rectangle; see set_frame().
 */
INLINE bool PGItem::
has_frame() const {
  LightReMutexHolder holder(_lock);
  return _has_frame;
}

/**
 * Removes the bounding rectangle from the item.  It will no longer be
 * possible to position the mouse within the item; see set_frame().
 */
INLINE void PGItem::
clear_frame() {
  LightReMutexHolder holder(_lock);
  if (_has_frame) {
    _has_frame = false;
    frame_changed();
  }
}

/**
 * Sets the "state" of this particular PGItem.
 *
 * The PGItem node will render as if it were the subgraph assigned to the
 * corresponding index via set_state_def().
 */
INLINE void PGItem::
set_state(int state) {
  LightReMutexHolder holder(_lock);
  _state = state;
}

/**
 * Returns the "state" of this particular PGItem.  See set_state().
 */
INLINE int PGItem::
get_state() const {
  LightReMutexHolder holder(_lock);
  return _state;
}

/**
 * Returns whether the PGItem is currently active for mouse events.  See
 * set_active().
 */
INLINE bool PGItem::
get_active() const {
  LightReMutexHolder holder(_lock);
  return (_flags & F_active) != 0;
}

/**
 * Returns whether the PGItem currently has focus for keyboard events.  See
 * set_focus().
 */
INLINE bool PGItem::
get_focus() const {
  LightReMutexHolder holder(_lock);
  return (_flags & F_focus) != 0;
}

/**
 * Returns whether background_focus is currently enabled.  See
 * set_background_focus().
 */
INLINE bool PGItem::
get_background_focus() const {
  LightReMutexHolder holder(_lock);
  return (_flags & F_background_focus) != 0;
}

/**
 * This is just an interface to set the suppress flags on the underlying
 * MouseWatcherRegion.  See MouseWatcherRegion::set_suppress_flags().
 */
INLINE void PGItem::
set_suppress_flags(int suppress_flags) {
  LightReMutexHolder holder(_lock);
  _region->set_suppress_flags(suppress_flags);
}

/**
 * This is just an interface to get the suppress flags on the underlying
 * MouseWatcherRegion.  See MouseWatcherRegion::get_suppress_flags().
 */
INLINE int PGItem::
get_suppress_flags() const {
  LightReMutexHolder holder(_lock);
  return _region->get_suppress_flags();
}

/**
 * Returns the Node that is the root of the subgraph that will be drawn when
 * the PGItem is in the indicated state.  The first time this is called for a
 * particular state index, it may create the Node.
 */
INLINE NodePath &PGItem::
get_state_def(int state) {
  nassertr(state >= 0 && state < 1000, get_state_def(0));  // Sanity check.

  LightReMutexHolder holder(_lock);
  return do_get_state_def(state);
}

/**
 * Returns the unique ID assigned to this PGItem.  This will be assigned to
 * the region created with the MouseWatcher, and will thus be used to generate
 * event names.
 */
INLINE const std::string &PGItem::
get_id() const {
  LightReMutexHolder holder(_lock);
  return _region->get_name();
}

/**
 * Set the unique ID assigned to this PGItem.  It is the user's responsibility
 * to ensure that this ID is unique.
 *
 * Normally, this should not need to be called, as the PGItem will assign
 * itself an ID when it is created, but this function allows the user to
 * decide to redefine the ID to be something possibly more meaningful.
 */
INLINE void PGItem::
set_id(const std::string &id) {
  LightReMutexHolder holder(_lock);
  _region->set_name(id);
}

/**
 * Returns the prefix that is used to define the enter event for all PGItems.
 * The enter event is the concatenation of this string followed by get_id().
 */
INLINE std::string PGItem::
get_enter_prefix() {
  return "enter-";
}

/**
 * Returns the prefix that is used to define the exit event for all PGItems.
 * The exit event is the concatenation of this string followed by get_id().
 */
INLINE std::string PGItem::
get_exit_prefix() {
  return "exit-";
}

/**
 * Returns the prefix that is used to define the within event for all PGItems.
 * The within event is the concatenation of this string followed by get_id().
 */
INLINE std::string PGItem::
get_within_prefix() {
  return "within-";
}

/**
 * Returns the prefix that is used to define the without event for all
 * PGItems.  The without event is the concatenation of this string followed by
 * get_id().
 */
INLINE std::string PGItem::
get_without_prefix() {
  return "without-";
}

/**
 * Returns the prefix that is used to define the focus_in event for all
 * PGItems.  The focus_in event is the concatenation of this string followed
 * by get_id().
 *
 * Unlike most item events, this event is thrown with no parameters.
 */
INLINE std::string PGItem::
get_focus_in_prefix() {
  return "fin-";
}

/**
 * Returns the prefix that is used to define the focus_out event for all
 * PGItems.  The focus_out event is the concatenation of this string followed
 * by get_id().
 *
 * Unlike most item events, this event is thrown with no parameters.
 */
INLINE std::string PGItem::
get_focus_out_prefix() {
  return "fout-";
}

/**
 * Returns the prefix that is used to define the press event for all PGItems.
 * The press event is the concatenation of this string followed by a button
 * name, followed by a hyphen and get_id().
 */
INLINE std::string PGItem::
get_press_prefix() {
  return "press-";
}

/**
 * Returns the prefix that is used to define the repeat event for all PGItems.
 * The repeat event is the concatenation of this string followed by a button
 * name, followed by a hyphen and get_id().
 */
INLINE std::string PGItem::
get_repeat_prefix() {
  return "repeat-";
}

/**
 * Returns the prefix that is used to define the release event for all
 * PGItems.  The release event is the concatenation of this string followed by
 * a button name, followed by a hyphen and get_id().
 */
INLINE std::string PGItem::
get_release_prefix() {
  return "release-";
}

/**
 * Returns the prefix that is used to define the keystroke event for all
 * PGItems.  The keystroke event is the concatenation of this string followed
 * by a hyphen and get_id().
 */
INLINE std::string PGItem::
get_keystroke_prefix() {
  return "keystroke-";
}

/**
 * Returns the event name that will be thrown when the item is active and the
 * mouse enters its frame, but not any nested frames.
 */
INLINE std::string PGItem::
get_enter_event() const {
  LightReMutexHolder holder(_lock);
  return get_enter_prefix() + get_id();
}

/**
 * Returns the event name that will be thrown when the item is active and the
 * mouse exits its frame, or enters a nested frame.
 */
INLINE std::string PGItem::
get_exit_event() const {
  LightReMutexHolder holder(_lock);
  return get_exit_prefix() + get_id();
}

/**
 * Returns the event name that will be thrown when the item is active and the
 * mouse moves within the boundaries of the frame.  This is different from the
 * enter_event in that the mouse is considered within the frame even if it is
 * also within a nested frame.
 */
INLINE std::string PGItem::
get_within_event() const {
  LightReMutexHolder holder(_lock);
  return get_within_prefix() + get_id();
}

/**
 * Returns the event name that will be thrown when the item is active and the
 * mouse moves completely outside the boundaries of the frame.  This is
 * different from the exit_event in that the mouse is considered within the
 * frame even if it is also within a nested frame.
 */
INLINE std::string PGItem::
get_without_event() const {
  LightReMutexHolder holder(_lock);
  return get_without_prefix() + get_id();
}

/**
 * Returns the event name that will be thrown when the item gets the keyboard
 * focus.
 */
INLINE std::string PGItem::
get_focus_in_event() const {
  LightReMutexHolder holder(_lock);
  return get_focus_in_prefix() + get_id();
}

/**
 * Returns the event name that will be thrown when the item loses the keyboard
 * focus.
 */
INLINE std::string PGItem::
get_focus_out_event() const {
  LightReMutexHolder holder(_lock);
  return get_focus_out_prefix() + get_id();
}

/**
 * Returns the event name that will be thrown when the item is active and the
 * indicated mouse or keyboard button is depressed while the mouse is within
 * the frame.
 */
INLINE std::string PGItem::
get_press_event(const ButtonHandle &button) const {
  LightReMutexHolder holder(_lock);
  return get_press_prefix() + button.get_name() + "-" + get_id();
}

/**
 * Returns the event name that will be thrown when the item is active and the
 * indicated mouse or keyboard button is continuously held down while the
 * mouse is within the frame.
 */
INLINE std::string PGItem::
get_repeat_event(const ButtonHandle &button) const {
  LightReMutexHolder holder(_lock);
  return get_repeat_prefix() + button.get_name() + "-" + get_id();
}

/**
 * Returns the event name that will be thrown when the item is active and the
 * indicated mouse or keyboard button, formerly clicked down is within the
 * frame, is released.
 */
INLINE std::string PGItem::
get_release_event(const ButtonHandle &button) const {
  LightReMutexHolder holder(_lock);
  return get_release_prefix() + button.get_name() + "-" + get_id();
}

/**
 * Returns the event name that will be thrown when the item is active and any
 * key is pressed by the user.
 */
INLINE std::string PGItem::
get_keystroke_event() const {
  LightReMutexHolder holder(_lock);
  return get_keystroke_prefix() + get_id();
}

/**
 * Changes the TextNode object that will be used by all PGItems to generate
 * default labels given a string.  This can be loaded with the default font,
 * etc.
 */
INLINE void PGItem::
set_text_node(TextNode *node) {
  _text_node = node;
}

/**
 * Returns the one PGItem in the world that currently has keyboard focus, if
 * any, or NULL if no item has keyboard focus.  Use PGItem::set_focus() to
 * activate or deactivate keyboard focus on a particular item.
 */
INLINE PGItem *PGItem::
get_focus_item() {
  return _focus_item;
}

/**
 * Returns the inverse of the frame transform matrix
 */
INLINE LMatrix4 PGItem::
get_frame_inv_xform() const {
  LightReMutexHolder holder(_lock);
  return _frame_inv_xform;
}

/**
 * Computes the area of the indicated frame.
 */
INLINE PN_stdfloat PGItem::
compute_area(const LVecBase4 &frame) {
  return (frame[1] - frame[0]) * (frame[3] - frame[2]);
}

/**
 * Given that largest is the pointer to the largest frame so far, and
 * largest_area is its area, compare that to the area of the new frame; if the
 * new frame is larger, adjust largest and largest_area appropriately.
 */
INLINE void PGItem::
compare_largest(const LVecBase4 *&largest, PN_stdfloat &largest_area,
                const LVecBase4 *new_frame) {
  PN_stdfloat new_area = compute_area(*new_frame);
  if (new_area > largest_area) {
    largest = new_frame;
    largest_area = new_area;
  }
}

/**
 *
 */
INLINE PGItem::StateDef::
StateDef() :
  _frame_stale(true)
{
}
