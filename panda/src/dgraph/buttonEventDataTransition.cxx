// Filename: buttonEventDataTransition.cxx
// Created by:  drose (09Feb99)
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

#if defined(WIN32_VC) && !defined(NO_PCH)
#include "dgraph_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "buttonEventDataTransition.h"
#include "buttonEventDataAttribute.h"
#endif

TypeHandle ButtonEventDataTransition::_type_handle;

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
//     Function: ButtonEventDataTransition::make_attrib
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeAttribute *ButtonEventDataTransition::
make_attrib() const {
  return new ButtonEventDataAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::compose
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
NodeTransition *ButtonEventDataTransition::
compose(const NodeTransition *) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::invert
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeTransition *ButtonEventDataTransition::
invert() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::apply
//       Access: Public, Virtual
//  Description: Returns a new attribute (or possibly the same
//               attribute) that represents the effect of applying this
//               indicated transition to the indicated attribute.  The
//               source attribute may be NULL, indicating the initial
//               attribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *ButtonEventDataTransition::
apply(const NodeAttribute *attrib) const {
  return (NodeAttribute *)attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventDataTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int ButtonEventDataTransition::
internal_compare_to(const NodeTransition *) const {
  return 0;
}
