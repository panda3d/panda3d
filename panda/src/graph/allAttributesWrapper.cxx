// Filename: allAttributesWrapper.cxx
// Created by:  drose (21Mar00)
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

#include "allAttributesWrapper.h"
#include "allTransitionsWrapper.h"

#include <indent.h>

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::apply_in_place
//       Access: Public
//  Description: Modifies the attribute by applying the transition.
////////////////////////////////////////////////////////////////////
void AllAttributesWrapper::
apply_in_place(const AllTransitionsWrapper &trans) {
  if (trans._cache != (NodeTransitionCache *)NULL) {
    _attrib.apply_in_place(*trans._cache);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::apply_from
//       Access: Public
//  Description: Modifies the attribute to reflect the application of
//               the indicated transition to the other attribute.
////////////////////////////////////////////////////////////////////
void AllAttributesWrapper::
apply_from(const AllAttributesWrapper &other,
           const AllTransitionsWrapper &trans) {
  if (trans._cache != (NodeTransitionCache *)NULL) {
    _attrib.apply_from(other._attrib, *trans._cache);
  } else {
    _attrib = other._attrib;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::apply
//       Access: Public
//  Description: Allocates and returns a new NodeAttributes object
//               that reflects the application of the transitions to
//               this wrapper.
////////////////////////////////////////////////////////////////////
NodeAttributes *AllAttributesWrapper::
apply(const AllTransitionsWrapper &trans) const {
  if (trans._cache != (NodeTransitionCache *)NULL) {
    return _attrib.apply(*trans._cache);
  } else {
    return new NodeAttributes(_attrib);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AllAttributesWrapper::
output(ostream &out) const {
  out << _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AllAttributesWrapper::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AllAttributesWrapper::
write(ostream &out, int indent_level) const {
  _attrib.write(out, indent_level);
}
