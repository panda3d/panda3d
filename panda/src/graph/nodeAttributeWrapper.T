// Filename: nodeAttributeWrapper.T
// Created by:  drose (20Mar00)
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


////////////////////////////////////////////////////////////////////
//     Function: get_attribute_into
//  Description: This external template function is handy for
//               extracting the attribute (of a known type) from the
//               wrapper.  If the attribute exists, it is
//               automatically downcasted to the correct type and
//               stored in the pointer given in the first parameter,
//               and the return value is true.  If the attribute does
//               not exist, the pointer is filled with NULL and the
//               return value is false.
////////////////////////////////////////////////////////////////////
template<class Attribute>
INLINE_GRAPH bool
get_attribute_into(Attribute *&ptr, const NodeAttributeWrapper &attrib) {
  NodeAttribute *nt = attrib.get_attrib();
  if (nt == (NodeAttribute *)NULL) {
    ptr = (Attribute *)NULL;
    return false;
  }
  DCAST_INTO_R(ptr, nt, false);
  return true;
}
