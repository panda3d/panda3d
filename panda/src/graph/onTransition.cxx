// Filename: onTransition.cxx
// Created by:  drose (22Mar00)
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

#include "onTransition.h"

#include <indent.h>

TypeHandle OnTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OnTransition::compose
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
NodeTransition *OnTransition::
compose(const NodeTransition *other) const {
  const OnTransition *ot;
  DCAST_INTO_R(ot, other, NULL);

  if (ot->_priority < _priority) {
    // The other transition is too low-priority; the result is
    // unchanged.
    return (OnTransition *)this;
  }

  // In any other case, for an OnTransition, the result is the same
  // as the second operand.
  return (OnTransition *)ot;
}

////////////////////////////////////////////////////////////////////
//     Function: OnTransition::invert
//       Access: Public, Virtual
//  Description: Returns a new transition that corresponds to the
//               inverse of this transition.  If the transition was
//               identity, this may return the same pointer.  Returns
//               NULL if the transition cannot be inverted.
////////////////////////////////////////////////////////////////////
NodeTransition *OnTransition::
invert() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: OnTransition::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void OnTransition::
output(ostream &out) const {
  output_value(out);
}

////////////////////////////////////////////////////////////////////
//     Function: OnTransition::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void OnTransition::
write(ostream &out, int indent_level) const {
  write_value(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: OnTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description: Returns a number < 0 if this transition sorts before
//               the other transition, > 0 if it sorts after, 0 if
//               they are equivalent (except for priority).
////////////////////////////////////////////////////////////////////
int OnTransition::
internal_compare_to(const NodeTransition *other) const {
  const OnTransition *ot;
  DCAST_INTO_R(ot, other, false);

  return compare_values(ot);
}
