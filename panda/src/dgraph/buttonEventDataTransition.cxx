// Filename: buttonEventDataTransition.cxx
// Created by:  drose (27Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "buttonEventDataTransition.h"
#include "buttonEventDataTransition.h"

#include "indent.h"
#include "modifierButtons.h"

#include <algorithm>

TypeHandle ButtonEventDataTransition::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::update_mods
//       Access: Public
//  Description: Updates the indicated ModifierButtons object with all
//               of the button up/down transitions indicated in the
//               queue.
////////////////////////////////////////////////////////////////////
void ButtonEventDataTransition::
update_mods(ModifierButtons &mods) const {
  Buttons::const_iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    mods.add_event(*bi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeTransition *ButtonEventDataTransition::
make_copy() const {
  return new ButtonEventDataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::compose
//       Access: Public, Virtual
//  Description: Attempts to compose this transition with the other one,
//               if that makes sense to do.  Returns a new
//               NodeTransition pointer that represents the compose, or
//               if the compose is not possible, returns the "other"
//               pointer unchanged (which is the result of the compose).
////////////////////////////////////////////////////////////////////
NodeTransition *ButtonEventDataTransition::
compose(const NodeTransition *other) const {
  const ButtonEventDataTransition *oa;
  DCAST_INTO_R(oa, other, NULL);

  if (_buttons.empty()) {
    return (ButtonEventDataTransition *)oa;

  } else if (oa->_buttons.empty()) {
    return (ButtonEventDataTransition *)this;

  } else {
    // We have to create a new data transition that includes both sets
    // of buttons.
    ButtonEventDataTransition *new_attrib =
      new ButtonEventDataTransition(*this);

    new_attrib->_buttons.insert(new_attrib->_buttons.end(),
                                oa->_buttons.begin(), oa->_buttons.end());
    return new_attrib;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::invert
//       Access: Public, Virtual
//  Description: Returns a new transition that corresponds to the
//               inverse of this transition.  If the transition was
//               identity, this may return the same pointer.  Returns
//               NULL if the transition cannot be inverted.
////////////////////////////////////////////////////////////////////
NodeTransition *ButtonEventDataTransition::
invert() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonEventDataTransition::
output(ostream &out) const {
  Buttons::const_iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    if (bi != _buttons.begin()) {
      out << ", ";
    }
    out << *bi;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonEventDataTransition::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << (*this) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int ButtonEventDataTransition::
internal_compare_to(const NodeTransition *other) const {
  const ButtonEventDataTransition *ot;
  DCAST_INTO_R(ot, other, false);

#ifdef WIN32_VC
  if (ot->_buttons == _buttons)
    return 0;
  else if (lexicographical_compare(_buttons.begin(), _buttons.end(),
                                ot->_buttons.begin(), ot->_buttons.end()))
    return -1;
  else
    return 1;
#else
  return lexicographical_compare_3way(_buttons.begin(), _buttons.end(),
                                      ot->_buttons.begin(), ot->_buttons.end());
#endif
}
