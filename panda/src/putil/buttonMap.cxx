// Filename: buttonMap.cxx
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

#include "buttonMap.h"

TypeHandle ButtonMap::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ButtonMap::map_button
//       Access: Public
//  Description: Registers a new button mapping.
////////////////////////////////////////////////////////////////////
void ButtonMap::
map_button(ButtonHandle raw_button, ButtonHandle button, const string &label) {
  int index = raw_button.get_index();
  if (_button_map.find(index) != _button_map.end()) {
    // A button with this index was already mapped.
    return;
  }

  ButtonNode bnode;
  bnode._raw = raw_button;
  bnode._mapped = button;
  bnode._label = label;
  _button_map[index] = bnode;
  _buttons.push_back(&_button_map[index]);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonMap::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void ButtonMap::
output(ostream &out) const {
  out << "ButtonMap (" << get_num_buttons() << " buttons)";
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonMap::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void ButtonMap::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "ButtonMap, " << get_num_buttons() << " buttons:\n";

  pvector<ButtonNode*>::const_iterator it;
  for (it = _buttons.begin(); it != _buttons.end(); ++it) {
    const ButtonNode *bn = *it;

    indent(out, indent_level + 2)
      << bn->_raw << " -> " << bn->_mapped << " \"" << bn->_label << "\"\n";
  }
}
