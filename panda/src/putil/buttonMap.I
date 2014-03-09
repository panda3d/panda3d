// Filename: buttonMap.I
// Created by:  rdb (09Mar14)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ButtonMap::get_num_buttons
//       Access: Published
//  Description: Returns the number of buttons that this button
//               mapping specifies.
////////////////////////////////////////////////////////////////////
INLINE int ButtonMap::
get_num_buttons() const {
  return _buttons.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonMap::get_raw_button
//       Access: Published
//  Description: Returns the underlying raw button associated with
//               the nth button.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle ButtonMap::
get_raw_button(int i) const {
  return _buttons[i]->_raw;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonMap::get_mapped_button
//       Access: Published
//  Description: Returns the nth mapped button, meaning the button
//               that the nth raw button is mapped to.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle ButtonMap::
get_mapped_button(int i) const {
  return _buttons[i]->_mapped;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonMap::get_mapped_button_label
//       Access: Published
//  Description: Returns the label associated with the nth mapped
//               button, meaning the button that the nth raw
//               button is mapped to.
////////////////////////////////////////////////////////////////////
INLINE const string &ButtonMap::
get_mapped_button_label(int i) const {
  return _buttons[i]->_label;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonMap::get_mapped_button
//       Access: Published
//  Description: Returns the button that the given button is mapped
//               to, or ButtonHandle::none() if this map does not
//               specify a mapped button for the given raw button.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: ButtonMap::get_mapped_button
//       Access: Published
//  Description: Returns the button that the given button is mapped
//               to, or ButtonHandle::none() if this map does not
//               specify a mapped button for the given raw button.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle ButtonMap::
get_mapped_button(const string &raw_name) const {
  ButtonHandle raw_button = ButtonRegistry::ptr()->find_button(raw_name);
  if (raw_button == ButtonHandle::none()) {
    return ButtonHandle::none();
  } else {
    return get_mapped_button(raw_button);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtoMap::get_mapped_button_label
//       Access: Published
//  Description: If the button map specifies a special name for the
//               button (eg. if the operating system or keyboard
//               device has a localized name describing the key),
//               returns it, or the empty string otherwise.
//
//               Note that this is not the same as
//               get_mapped_button().get_name(), which returns the
//               name of the Panda event associated with the button.
////////////////////////////////////////////////////////////////////
INLINE const string &ButtonMap::
get_mapped_button_label(ButtonHandle raw) const {
  pmap<int, ButtonNode>::const_iterator it;
  it = _button_map.find(raw.get_index());
  if (it == _button_map.end()) {
    static const string empty = "";
    return empty;
  } else {
    return it->second._label;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtoMap::get_mapped_button_label
//       Access: Published
//  Description: If the button map specifies a special name for the
//               button (eg. if the operating system or keyboard
//               device has a localized name describing the key),
//               returns it, or the empty string otherwise.
//
//               Note that this is not the same as
//               get_mapped_button().get_name(), which returns the
//               name of the Panda event associated with the button.
////////////////////////////////////////////////////////////////////
INLINE const string &ButtonMap::
get_mapped_button_label(const string &raw_name) const {
  ButtonHandle raw_button = ButtonRegistry::ptr()->find_button(raw_name);
  if (raw_button == ButtonHandle::none()) {
    static const string empty = "";
    return empty;
  } else {
    return get_mapped_button_label(raw_button);
  }
}
