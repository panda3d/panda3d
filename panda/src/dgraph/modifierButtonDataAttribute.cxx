// Filename: modifierButtonDataAttribute.cxx
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "modifierButtonDataAttribute.h"
#include "modifierButtonDataTransition.h"

#include <indent.h>

TypeHandle ModifierButtonDataAttribute::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataAttribute::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *ModifierButtonDataAttribute::
make_copy() const {
  return new ModifierButtonDataAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataAttribute::make_initial
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *ModifierButtonDataAttribute::
make_initial() const {
  return new ModifierButtonDataAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataAttribute::get_handle
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TypeHandle ModifierButtonDataAttribute::
get_handle() const {
  return ModifierButtonDataTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataAttribute::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ModifierButtonDataAttribute::
output(ostream &out) const {
  out << _mods;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataAttribute::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ModifierButtonDataAttribute::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _mods << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataAttribute::internal_compare_to
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int ModifierButtonDataAttribute::
internal_compare_to(const NodeAttribute *other) const {
  const ModifierButtonDataAttribute *ot;
  DCAST_INTO_R(ot, other, false);

  if (_mods != ot->_mods) {
    return (_mods < ot->_mods) ? -1 : 1;
  }
  return 0;
}
