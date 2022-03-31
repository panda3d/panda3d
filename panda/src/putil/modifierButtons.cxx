/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modifierButtons.cxx
 * @author drose
 * @date 2000-03-01
 */

#include "modifierButtons.h"

#include "pnotify.h"

/**
 *
 */
ModifierButtons::
ModifierButtons() :
  _state(0)
{
   _button_list = PTA(ButtonHandle)::empty_array(0);
}

/**
 *
 */
ModifierButtons::
ModifierButtons(const ModifierButtons &copy) :
  _button_list(copy._button_list),
  _state(copy._state)
{
}

/**
 *
 */
ModifierButtons::
~ModifierButtons() {
}

/**
 * Sets is_down() true for any button that is already true for this object and
 * the other object.
 */
void ModifierButtons::
operator &= (const ModifierButtons &other) {
  if (_button_list == other._button_list) {
    // Trivially easy case: if the button lists are the same, we can do this
    // using a bitmask operation.
    _state &= other._state;

  } else {
    // More complicated case: if the button lists are different, we have to
    // iterate through the buttons and compare them case-by-case.  This
    // becomes an n^2 operation, but fortunately there won't be more than a
    // handful of buttons.
    int num_buttons = get_num_buttons();
    for (int i = 0; i < num_buttons; i++) {
      if (is_down(i) && !other.is_down(get_button(i))) {
        _state &= ~((BitmaskType)1 << i);
      }
    }
  }
}

/**
 * Sets is_down() true for any button that is already true for this object and
 * the other object.  Adds whatever buttons are necessary to the list to make
 * this so
 */
void ModifierButtons::
operator |= (const ModifierButtons &other) {
  if (_button_list == other._button_list) {
    // Trivially easy case: if the button lists are the same, we can do this
    // using a bitmask operation.
    _state |= other._state;

  } else {
    // More complicated case: if the button lists are different, we have to
    // iterate through the buttons and compare them case-by-case.  This
    // becomes an n^2 operation, but fortunately there won't be more than a
    // handful of buttons.
    int num_buttons = other.get_num_buttons();
    for (int i = 0; i < num_buttons; i++) {
      if (other.is_down(i)) {
        add_button(other.get_button(i));
        button_down(other.get_button(i));
      }
    }
  }
}

/**
 * Sets the list of buttons to watch to be the same as that of the other
 * ModifierButtons object.  This makes the lists pointer equivalent (until one
 * or the other is later modified).
 *
 * This will preserve the state of any button that was on the original list
 * and is also on the new lists.  Any other buttons will get reset to the
 * default state of "up".
 */
void ModifierButtons::
set_button_list(const ModifierButtons &other) {
  if (_button_list != other._button_list) {
    if (_state != 0) {
      // If we have some buttons already down, we have to copy them to the new
      // state.
      BitmaskType new_state = 0;
      int num_buttons = other.get_num_buttons();
      for (int i = 0; i < num_buttons; i++) {
        if (is_down(other.get_button(i))) {
          new_state |= ((BitmaskType)1 << i);
        }
      }

      _state = new_state;
    }

    _button_list = other._button_list;
  }
}

/**
 * Returns true if the set of buttons indicated as down by this
 * ModifierButtons object is the same set of buttons indicated as down by the
 * other ModifierButtons object.  The buttons indicated as up are not
 * relevant.
 */
bool ModifierButtons::
matches(const ModifierButtons &other) const {
  if (_button_list == other._button_list) {
    // If the two objects share the same array, we only need to check the
    // bitmask.  This is a simple optimization.
    return (_state == other._state);
  }

  // The two objects do not share the same array; thus we have to do this one
  // button at a time.  This is an n-squared operation, but presumably there
  // will not be hundreds of buttons to compare.

  // First, check that all the buttons indicated as down in our object are
  // also indicated as down in the other object.
  int num_down = 0;

  int i;
  for (i = 0; i < (int)_button_list.size(); i++) {
    if (is_down(i)) {
      if (!other.is_down(_button_list[i])) {
        return false;
      }
      num_down++;
    }
  }

  // Now make sure the total number of buttons indicated as down in our object
  // matches the number indicated as down in the other object.  This ensures
  // there aren't any additional buttons indicated down in the other object.
  int num_other_buttons = other.get_num_buttons();
  int num_other_down = 0;
  for (i = 0; i < num_other_buttons; i++) {
    if (other.is_down(i)) {
      num_other_down++;
    }
  }

  return (num_down == num_other_down);
}

/**
 * Adds the indicated button to the set of buttons that will be monitored for
 * upness and downness.  Returns true if the button was added, false if it was
 * already being monitored or if too many buttons are currently being
 * monitored.
 */
bool ModifierButtons::
add_button(ButtonHandle button) {
  nassertr(button != ButtonHandle::none(), false);

  static const int max_buttons = sizeof(BitmaskType) * 8;

  if ((int)_button_list.size() >= max_buttons) {
    return false;
  }

  // First, check to see if the button is already being monitored.
  if (has_button(button)) {
    return false;
  }

  // Ok, it's not; add it.
  modify_button_list();
  _button_list.push_back(button);

  return true;
}

/**
 * Returns true if the indicated button is in the set of buttons being
 * monitored, false otherwise.
 */
bool ModifierButtons::
has_button(ButtonHandle button) const {
  PTA(ButtonHandle)::const_iterator bi;
  for (bi = _button_list.begin(); bi != _button_list.end(); ++bi) {
    if (button.matches(*bi)) {
      return true;
    }
  }

  return false;
}

/**
 * Removes the indicated button from the set of buttons being monitored.
 * Returns true if the button was removed, false if it was not being monitored
 * in the first place.
 *
 * Unlike the other methods, you cannot remove a button by removing its alias;
 * you have to remove exactly the button itself.
 */
bool ModifierButtons::
remove_button(ButtonHandle button) {
  // We use i instead of an iterator, because we need to call
  // modify_button_list() just before we remove the button, and that may
  // invalidate all of the iterators.

  for (int i = 0; i < (int)_button_list.size(); i++) {
    if (button == _button_list[i]) {
      modify_button_list();
      _button_list.erase(_button_list.begin() + i);

      // Now remove the corresponding bit from the bitmask and shift all the
      // bits above it down.
      BitmaskType mask = ((BitmaskType)1 << i);
      BitmaskType below = mask - 1;
      BitmaskType above = (~below) & (~mask);

      _state = ((_state & above) >> 1) | (_state & below);
      return true;
    }
  }

  return false;
}

/**
 * Records that a particular button has been pressed.  If the given button is
 * one of the buttons that is currently being monitored, this will update the
 * internal state appropriately; otherwise, it will do nothing.  Returns true
 * if the button is one that was monitored, or false otherwise.
 */
bool ModifierButtons::
button_down(ButtonHandle button) {
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if (button.matches(_button_list[i])) {
      _state |= ((BitmaskType)1 << i);
      return true;
    }
  }

  return false;
}

/**
 * Records that a particular button has been released.  If the given button is
 * one of the buttons that is currently being monitored, this will update the
 * internal state appropriately; otherwise, it will do nothing.  Returns true
 * if the button is one that was monitored, or false otherwise.
 */
bool ModifierButtons::
button_up(ButtonHandle button) {
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if (button.matches(_button_list[i])) {
      _state &= ~((BitmaskType)1 << i);
      return true;
    }
  }

  return false;
}

/**
 * Returns true if the indicated button is known to be down, or false if it is
 * known to be up or if it is not in the set of buttons being tracked.
 */
bool ModifierButtons::
is_down(ButtonHandle button) const {
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if (button.matches(_button_list[i])) {
      return ((_state & ((BitmaskType)1 << i)) != 0);
    }
  }

  return false;
}

/**
 * Returns a string which can be used to prefix any button name or event name
 * with the unique set of modifier buttons currently being held.
 */
std::string ModifierButtons::
get_prefix() const {
  std::string prefix;
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if ((_state & ((BitmaskType)1 << i)) != 0) {
      prefix += _button_list[i].get_name();
      prefix += '-';
    }
  }

  return prefix;
}

/**
 * Writes a one-line summary of the buttons known to be down.
 */
void ModifierButtons::
output(std::ostream &out) const {
  out << "[";
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if ((_state & ((BitmaskType)1 << i)) != 0) {
      out << " " << _button_list[i];
    }
  }
  out << " ]";
}

/**
 * Writes a multi-line summary including all of the buttons being monitored
 * and which ones are known to be down.
 */
void ModifierButtons::
write(std::ostream &out) const {
  out << "ModifierButtons:\n";
  for (int i = 0; i < (int)_button_list.size(); i++) {
    out << "  " << _button_list[i];
    if ((_state & ((BitmaskType)1 << i)) != 0) {
      out << " (down)";
    }
    out << "\n";
  }
}

/**
 * Implements a poor-man's copy-on-write for the ModifierButtons class.  If
 * any reference counts are held on our _button_list, besides ourselves, then
 * allocates and copies a brand new copy of the _button_list.  This should be
 * done in preparation for any modifications to the _button_list, since
 * multiple instances of the ModifierButtons object may share the same
 * _button_list pointer.
 */
void ModifierButtons::
modify_button_list() {
  if (_button_list.get_ref_count() > 1) {
    PTA(ButtonHandle) old_list = _button_list;

    _button_list = PTA(ButtonHandle)::empty_array(0);

    // This forces a new allocation and memberwise copy, instead of just a
    // reference-counting pointer copy.
    _button_list.v() = old_list.v();
  }

  // Now we should be the only ones holding a count.
  nassertv(_button_list.get_ref_count() == 1);
}
