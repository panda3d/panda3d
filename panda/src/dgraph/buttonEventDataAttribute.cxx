// Filename: buttonEventDataAttribute.cxx
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

#include "buttonEventDataAttribute.h"
#include "buttonEventDataTransition.h"

#include <indent.h>
#include <modifierButtons.h>
#include <algorithm>

TypeHandle ButtonEventDataAttribute::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::update_mods
//       Access: Public
//  Description: Updates the indicated ModifierButtons object with all
//               of the button up/down transitions indicated in the
//               queue.
////////////////////////////////////////////////////////////////////
void ButtonEventDataAttribute::
update_mods(ModifierButtons &mods) const {
  Buttons::const_iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    mods.add_event(*bi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *ButtonEventDataAttribute::
make_copy() const {
  return new ButtonEventDataAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::make_initial
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *ButtonEventDataAttribute::
make_initial() const {
  return new ButtonEventDataAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::get_handle
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TypeHandle ButtonEventDataAttribute::
get_handle() const {
  return ButtonEventDataTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::merge
//       Access: Public, Virtual
//  Description: Attempts to merge this attribute with the other one,
//               if that makes sense to do.  Returns a new
//               NodeAttribute pointer that represents the merge, or
//               if the merge is not possible, returns the "other"
//               pointer unchanged (which is the result of the merge).
////////////////////////////////////////////////////////////////////
NodeAttribute *ButtonEventDataAttribute::
merge(const NodeAttribute *other) const {
  const ButtonEventDataAttribute *oa;
  DCAST_INTO_R(oa, other, NULL);

  if (_buttons.empty()) {
    return (ButtonEventDataAttribute *)oa;

  } else if (oa->_buttons.empty()) {
    return (ButtonEventDataAttribute *)this;

  } else {
    // We have to create a new data attribute that includes both sets
    // of buttons.
    ButtonEventDataAttribute *new_attrib =
      new ButtonEventDataAttribute(*this);

    new_attrib->_buttons.insert(new_attrib->_buttons.end(),
                                oa->_buttons.begin(), oa->_buttons.end());
    return new_attrib;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonEventDataAttribute::
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
//     Function: ButtonEventDataAttribute::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonEventDataAttribute::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << (*this) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataAttribute::internal_compare_to
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int ButtonEventDataAttribute::
internal_compare_to(const NodeAttribute *other) const {
  const ButtonEventDataAttribute *ot;
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
