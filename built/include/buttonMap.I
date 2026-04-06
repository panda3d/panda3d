/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonMap.I
 * @author rdb
 * @date 2014-03-09
 */

/**
 * Returns the number of buttons that this button mapping specifies.
 */
INLINE size_t ButtonMap::
get_num_buttons() const {
  return _buttons.size();
}

/**
 * Returns the underlying raw button associated with the nth button.
 */
INLINE ButtonHandle ButtonMap::
get_raw_button(size_t i) const {
  return _buttons[i]->_raw;
}

/**
 * Returns the nth mapped button, meaning the button that the nth raw button
 * is mapped to.
 */
INLINE ButtonHandle ButtonMap::
get_mapped_button(size_t i) const {
  return _buttons[i]->_mapped;
}

/**
 * Returns the label associated with the nth mapped button, meaning the button
 * that the nth raw button is mapped to.
 */
INLINE const std::string &ButtonMap::
get_mapped_button_label(size_t i) const {
  return _buttons[i]->_label;
}

/**
 * Returns the button that the given button is mapped to, or
 * ButtonHandle::none() if this map does not specify a mapped button for the
 * given raw button.
 */
INLINE ButtonHandle ButtonMap::
get_mapped_button(ButtonHandle raw) const {
  pmap<int, ButtonNode>::const_iterator it;
  it = _button_map.find(raw.get_index());
  if (it == _button_map.end()) {
    return ButtonHandle::none();
  } else {
    return it->second._mapped;
  }
}

/**
 * Returns the button that the given button is mapped to, or
 * ButtonHandle::none() if this map does not specify a mapped button for the
 * given raw button.
 */
INLINE ButtonHandle ButtonMap::
get_mapped_button(const std::string &raw_name) const {
  ButtonHandle raw_button = ButtonRegistry::ptr()->find_button(raw_name);
  if (raw_button == ButtonHandle::none()) {
    return ButtonHandle::none();
  } else {
    return get_mapped_button(raw_button);
  }
}

/**
 * If the button map specifies a special name for the button (eg.  if the
 * operating system or keyboard device has a localized name describing the
 * key), returns it, or the empty string otherwise.
 *
 * Note that this is not the same as get_mapped_button().get_name(), which
 * returns the name of the Panda event associated with the button.
 */
INLINE const std::string &ButtonMap::
get_mapped_button_label(ButtonHandle raw) const {
  pmap<int, ButtonNode>::const_iterator it;
  it = _button_map.find(raw.get_index());
  if (it == _button_map.end()) {
    static const std::string empty = "";
    return empty;
  } else {
    return it->second._label;
  }
}

/**
 * If the button map specifies a special name for the button (eg.  if the
 * operating system or keyboard device has a localized name describing the
 * key), returns it, or the empty string otherwise.
 *
 * Note that this is not the same as get_mapped_button().get_name(), which
 * returns the name of the Panda event associated with the button.
 */
INLINE const std::string &ButtonMap::
get_mapped_button_label(const std::string &raw_name) const {
  ButtonHandle raw_button = ButtonRegistry::ptr()->find_button(raw_name);
  if (raw_button == ButtonHandle::none()) {
    static const std::string empty = "";
    return empty;
  } else {
    return get_mapped_button_label(raw_button);
  }
}
