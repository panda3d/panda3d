// Filename: modifierButtonDataTransition.cxx
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "modifierButtonDataTransition.h"
#include "modifierButtonDataAttribute.h"

TypeHandle ModifierButtonDataTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataTransition::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransition *ModifierButtonDataTransition::
make_copy() const {
  return new ModifierButtonDataTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataTransition::make_attrib
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeAttribute *ModifierButtonDataTransition::
make_attrib() const {
  return new ModifierButtonDataAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataTransition::compose
//       Access: Public, Virtual
//  Description: Returns a new transition that corresponds to the
//               composition of this transition with the second
//               transition (which must be of an equivalent type).
//               This may return the same pointer as either source
//               transition.  Applying the transition returned from
//               this function to an attribute attribute will produce
//               the same effect as applying each transition
//               separately.
////////////////////////////////////////////////////////////////////
NodeTransition *ModifierButtonDataTransition::
compose(const NodeTransition *) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataTransition::invert
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransition *ModifierButtonDataTransition::
invert() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataTransition::apply
//       Access: Public, Virtual
//  Description: Returns a new attribute (or possibly the same
//               attribute) that represents the effect of applying this
//               indicated transition to the indicated attribute.  The
//               source attribute may be NULL, indicating the initial
//               attribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *ModifierButtonDataTransition::
apply(const NodeAttribute *attrib) const {
  return (NodeAttribute *)attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtonDataTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int ModifierButtonDataTransition::
internal_compare_to(const NodeTransition *) const {
  return 0;
}
