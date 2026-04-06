/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modifierButtons.I
 * @author drose
 * @date 2000-03-01
 */

/**
 *
 */
INLINE void ModifierButtons::
operator = (const ModifierButtons &copy) {
  _button_list = copy._button_list;
  _state = copy._state;
}

/**
 * The equality operator is an exact comparision: the two ModifierButtons are
 * equal if they share the same button list--indeed, the same pointer--and
 * they all the buttons have the same state.  Use matches() if a less exact
 * equality test is needed.
 */
INLINE bool ModifierButtons::
operator == (const ModifierButtons &other) const {
  return (_button_list == other._button_list &&
          _state == other._state);
}

/**
 *
 */
INLINE bool ModifierButtons::
operator != (const ModifierButtons &other) const {
  return !operator == (other);
}

/**
 *
 */
INLINE bool ModifierButtons::
operator < (const ModifierButtons &other) const {
  if (_button_list != other._button_list) {
    return _button_list < other._button_list;
  }
  return _state < other._state;
}

/**
 * Returns a new ModifierButtons object for which is_down() will be true only
 * if it is true on both source objects.  The set of buttons reported by
 * has_button() is not completely defined if both source objects have a
 * different set.
 */
INLINE ModifierButtons ModifierButtons::
operator & (const ModifierButtons &other) const {
  ModifierButtons result = *this;
  result &= other;
  return result;
}


/**
 * Returns a new ModifierButtons object for which is_down() will be true if it
 * is true on either of the source objects.  The set of buttons reported by
 * has_button() is not completely defined if both source objects have a
 * different set.
 */
INLINE ModifierButtons ModifierButtons::
operator | (const ModifierButtons &other) const {
  ModifierButtons result = *this;
  result |= other;
  return result;
}

/**
 * Returns the number of buttons that the ModifierButtons object is monitoring
 * (e.g.  the number of buttons passed to add_button()).
 */
INLINE int ModifierButtons::
get_num_buttons() const {
  return _button_list.size();
}

/**
 * Returns the nth button that the ModifierButtons object is monitoring (the
 * nth button passed to add_button()).  This must be in the range 0 <= index <
 * get_num_buttons().
 */
INLINE ButtonHandle ModifierButtons::
get_button(int index) const {
  nassertr(index >= 0 && index < (int)_button_list.size(), ButtonHandle::none());
  return _button_list[index];
}

/**
 * Marks all monitored buttons as being in the "up" state.
 */
INLINE void ModifierButtons::
all_buttons_up() {
  _state = 0;
}

/**
 * Returns true if the indicated button is known to be down, or false if it is
 * known to be up.
 */
INLINE bool ModifierButtons::
is_down(int index) const {
  nassertr(index >= 0 && index < (int)_button_list.size(), false);
  return ((_state & ((BitmaskType)1 << index)) != 0);
}

/**
 * Returns true if any of the tracked button are known to be down, or false if
 * all of them are up.
 */
INLINE bool ModifierButtons::
is_any_down() const {
  return _state != 0;
}
