// Filename: immediateTransition.cxx
// Created by:  drose (24Mar00)
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

#include "immediateTransition.h"
#include "immediateAttribute.h"

#include <indent.h>

TypeHandle ImmediateTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ImmediateTransition::make_attrib
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *ImmediateTransition::
make_attrib() const {
  return new ImmediateAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ImmediateTransition::compose
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
NodeTransition *ImmediateTransition::
compose(const NodeTransition *) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ImmediateTransition::invert
//       Access: Public, Virtual
//  Description: Returns a new transition that corresponds to the
//               inverse of this transition.  If the transition was
//               identity, this may return the same pointer.  Returns
//               NULL if the transition cannot be inverted.
////////////////////////////////////////////////////////////////////
NodeTransition *ImmediateTransition::
invert() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ImmediateTransition::apply
//       Access: Public, Virtual
//  Description: Returns a new attribute (or possibly the same
//               attribute) that represents the effect of applying this
//               indicated transition to the indicated attribute.  The
//               source attribute may be NULL, indicating the initial
//               attribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *ImmediateTransition::
apply(const NodeAttribute *) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ImmediateTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int ImmediateTransition::
internal_compare_to(const NodeTransition *) const {
  return 0;
}
