// Filename: buttonEventDataAttribute.cxx
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "buttonEventDataAttribute.h"
#include "buttonEventDataTransition.h"

#include <indent.h>

#include <algorithm>

TypeHandle ButtonEventDataAttribute::_type_handle;


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
